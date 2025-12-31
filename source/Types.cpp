module;

#include <quill/LogMacros.h>
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
	vk::Format imageFormat_;
	std::vector<vk::Image> images_;
	std::vector<vk::raii::ImageView> imageViews_;
	std::vector<vk::raii::Semaphore> readyToPresent_;

	explicit SwapchainData(vk::raii::PhysicalDevice const& gpu,
	                       vk::raii::Device const& logical_device,
	                       vk::raii::SurfaceKHR const& surface,
	                       vk::PresentModeKHR const& present_mode,
	                       vk::Extent2D const& window_extent) :
	    bootstrapSwapchain_{makeVkbSwapchain(gpu, logical_device, surface, present_mode, window_extent)},
	    swapchain_{logical_device, bootstrapSwapchain_.swapchain},
	    imageFormat_{bootstrapSwapchain_.image_format},
	    images_{swapchain_.getImages().value},
	    imageViews_{makeSwapchainImageViews(logical_device)},
	    readyToPresent_{makeReadyToPresentSemaphores(logical_device, images_)}
	{}

private:
	static auto makeVkbSwapchain(vk::raii::PhysicalDevice const& gpu,
	                             vk::raii::Device const& device,
	                             vk::raii::SurfaceKHR const& surface,
	                             vk::PresentModeKHR const& present_mode,
	                             vk::Extent2D const& window_extent) -> vkb::Swapchain
	{
		constexpr auto surface_format =
		    vk::SurfaceFormatKHR{.format = vk::Format::eB8G8R8A8Unorm, .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear};

		auto const builder =
		    vkb::SwapchainBuilder{*gpu, *device, *surface}
		        .set_desired_format(surface_format)
		        .set_desired_present_mode(static_cast<VkPresentModeKHR>(present_mode))
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

	static auto makeReadyToPresentSemaphores(vk::raii::Device const& logical_device,
	                                         std::vector<vk::Image> const& swapchain_images)
	    -> std::vector<vk::raii::Semaphore>
	{
		auto ready_to_present_semaphores = std::vector<vk::raii::Semaphore>{};
		ready_to_present_semaphores.reserve(swapchain_images.size());
		std::generate_n(std::back_inserter(ready_to_present_semaphores), swapchain_images.size(),
		                        [&logical_device] { return logical_device.createSemaphore({}).value; });
		return ready_to_present_semaphores;
	}
};

struct FrameCommand
{
	static constexpr auto FRAME_OVERLAP = 3U;
	using TripleBufferedFrames = std::array<FrameCommand, FRAME_OVERLAP>;

	vk::raii::CommandPool commandPool_;
	vk::raii::CommandBuffer commandBuffer_;
	vk::raii::Fence renderFence_;
	vk::raii::Semaphore imageAcquired_;

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
	    // One fence to block until device has finished rendering; host will record buffers/do other things
	    // Semaphore synchronises 
	    renderFence_{logical_device.createFence({.flags = vk::FenceCreateFlagBits::eSignaled}).value},
	    imageAcquired_{logical_device.createSemaphore({}).value}
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
