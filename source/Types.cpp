module;

#include "quill/LogMacros.h"

#include <VkBootstrap.h>

export module Caldera:Types;
import :Util;

import vulkan;

export namespace caldera
{
// Swapchain data; collects all the swapchain-related stuff together
struct SwapchainData
{
	vkb::Swapchain bootstrapSwapchain_;
	vk::raii::SwapchainKHR swapchain_;
	vk::Format swapchainImageFormat_;
	std::vector<vk::Image> images_;
	std::vector<vk::raii::ImageView> imageViews_;

	explicit SwapchainData(vk::raii::PhysicalDevice const& gpu,
	                       vk::raii::Device const& logical_device,
	                       vk::raii::SurfaceKHR const& surface,
	                       vk::Extent2D const& window_extent) :
	    bootstrapSwapchain_{makeVkbSwapchain(gpu, logical_device, surface, window_extent)},
	    swapchain_{logical_device, bootstrapSwapchain_.swapchain},
	    swapchainImageFormat_{bootstrapSwapchain_.image_format},
	    images_{swapchain_.getImages().value},
	    imageViews_{makeSwapchainImageViews(logical_device)}
	{}

private:
	static auto makeVkbSwapchain(vk::raii::PhysicalDevice const& gpu,
	                             vk::raii::Device const& device,
	                             vk::raii::SurfaceKHR const& surface,
	                             vk::Extent2D const& window_extent) -> vkb::Swapchain
	{
		constexpr auto surface_format =
		    vk::SurfaceFormatKHR{.format = vk::Format::eB8G8R8A8Unorm, .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear};

		auto const builder =
		    vkb::SwapchainBuilder{*gpu, *device, *surface}
		        .set_desired_format(surface_format)
		        .set_desired_present_mode(static_cast<VkPresentModeKHR>(vk::PresentModeKHR::eFifo))
		        .set_desired_extent(window_extent.width, window_extent.height)
		        .add_image_usage_flags(static_cast<VkImageUsageFlags>(vk::ImageUsageFlagBits::eTransferDst))
		        .build();

		if (not builder.has_value()) {
			LOG_ERROR(logger, "Could not create swapchain: {}", builder.error().message());
			std::exit(EXIT_FAILURE);
		}
		return builder.value();
	}

	auto makeSwapchainImageViews(vk::raii::Device const& device) -> decltype(imageViews_)
	{
		return bootstrapSwapchain_.get_image_views().value()
		       | std::views::transform(
		           [&device](auto&& image_view) -> vk::raii::ImageView { return {device, image_view}; })
		       | std::ranges::to<decltype(imageViews_)>();
	}
};

struct FrameCommand
{
	static constexpr auto FRAME_OVERLAP = 3U;
	using TripleBufferedFrames = std::array<FrameCommand, FRAME_OVERLAP>;

	vk::raii::CommandPool commandPool_;
	vk::raii::CommandBuffer commandBuffer_;
	vk::raii::Semaphore swapchainSemaphore_;
	vk::raii::Semaphore renderSemaphore_;
	vk::raii::Fence renderFence_;

	explicit FrameCommand(
	    vk::raii::Device const& logical_device,
	    std::uint32_t const queue_family_index,
	    vk::CommandPoolCreateFlagBits const flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer) :
	    commandPool_(logical_device.createCommandPool({.flags = flags, .queueFamilyIndex = queue_family_index}).value),
	    commandBuffer_{std::move(
	        logical_device
	            .allocateCommandBuffers(
	                {.commandPool = commandPool_, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1U})
	            .value.front())},
	    swapchainSemaphore_{logical_device.createSemaphore({}).value},
	    renderSemaphore_{logical_device.createSemaphore({}).value},
	    renderFence_{logical_device.createFence({.flags = vk::FenceCreateFlagBits::eSignaled}).value}
	{}

	static auto
	makeTripleBufferedFrames(vk::raii::Device const& logical_device,
	                         std::uint32_t queue_family_index,
	                         vk::CommandPoolCreateFlagBits flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
	    -> TripleBufferedFrames
	{
		auto const make_triple_buffered_frames = [&logical_device, queue_family_index, flags] -> auto {
			return FrameCommand{logical_device, queue_family_index, flags};
		};
		return TripleBufferedFrames{make_triple_buffered_frames(), make_triple_buffered_frames(),
		                            make_triple_buffered_frames()};
	}
};
} // namespace caldera
