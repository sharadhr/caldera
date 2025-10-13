module;

#include <quill/LogMacros.h>

module Caldera:Types;

import :Initialisers;
import :Logging;

import std;
import vk_mem_alloc_hpp;
import vulkan_hpp;

namespace caldera
{
struct FrameCommand
{
	static constexpr auto FRAME_OVERLAP = 3U;
	using TripleBuffered = std::array<FrameCommand, FRAME_OVERLAP>;

	explicit FrameCommand(vk::raii::Device const& logical_device,
	                      std::uint32_t queue_family_index,
	                      vk::CommandPoolCreateFlagBits flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer) :
		commandPool_{(logical_device.createCommandPool({.flags = flags, .queueFamilyIndex = queue_family_index}).value())},
		mainCommandBuffer_{
			std::move(logical_device
	                .allocateCommandBuffers(
										{.commandPool = commandPool_, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1U})
	                .value()
	                .front())},
		swapchainSemaphore_{(logical_device.createSemaphore({}).value())},
		renderSemaphore_{(logical_device.createSemaphore({}).value())},
		renderFence_{(logical_device.createFence({.flags = vk::FenceCreateFlagBits::eSignaled}).value())}
	{}

	static auto
	makeFrameCommands(vk::raii::Device const& logical_device,
	                  std::uint32_t queue_family_index,
	                  vk::CommandPoolCreateFlagBits flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		-> TripleBuffered
	{
		// there's probably a nice template way to produce an array from a range
		return TripleBuffered{FrameCommand(logical_device, queue_family_index, flags),
		                      FrameCommand(logical_device, queue_family_index, flags),
		                      FrameCommand(logical_device, queue_family_index, flags)};
	}

	vk::raii::CommandPool commandPool_;
	vk::raii::CommandBuffer mainCommandBuffer_;
	vk::raii::Semaphore swapchainSemaphore_;
	vk::raii::Semaphore renderSemaphore_;
	vk::raii::Fence renderFence_;
};

struct AllocatedImage
{
	AllocatedImage(vk::Extent3D const& extent,
	               vk::Format const& format,
	               vk::raii::Device const& logical_device,
	               vma::Allocator const& allocator,
	               vk::ImageUsageFlags const& image_usage_flags) :
		extent_{extent},
		format_{format},
		allocator{allocator},
		imageAndMemory_{
			allocator
				.createImage(init::makeImageCreateInfo(format_, extent_, image_usage_flags),
	                   {.usage = vma::MemoryUsage::eGpuOnly, .requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal})
				.value},
		view_{
			logical_device
				.createImageView(init::makeImageViewCreateInfo(format_, imageAndMemory_.first, vk::ImageAspectFlagBits::eColor))
				.value()}
	{}

	~AllocatedImage() { allocator.destroyImage(imageAndMemory_.first, imageAndMemory_.second); }

private:
	using VMAImageAndMemory =
		std::unique_ptr<std::pair<vk::Image, vma::Allocation>, decltype(vma::Allocator::destroyImage)>;

public:
	vk::Extent3D extent_;
	vk::Format format_;
	vma::Allocator const& allocator;
	std::pair<vk::Image, vma::Allocation> imageAndMemory_;
	vk::raii::ImageView view_{nullptr};
};

constexpr inline auto checkResult = [](vk::Result const& result) {
	if (result != vk::Result::eSuccess) {
		LOG_ERROR(caldera::log::logger, "Vulkan error: {}", vk::to_string(result));
	}
	return result == vk::Result::eSuccess;
};
} // namespace caldera
