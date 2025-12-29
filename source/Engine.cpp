module;

#include <quill/LogMacros.h>
#include <quill/std/Vector.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <VkBootstrap.h>

module Caldera;

import :Util;
import :Images;
import vk_mem_alloc;
import vulkan;

using namespace std::literals;

constexpr auto HUNDRED_MILLISECONDS = 100ms;
constexpr auto NANOSECONDS_IN_A_SECOND = std::chrono::nanoseconds{1s};

namespace caldera
{
Engine::~Engine() { check_if_success(logicalDevice_.waitIdle()); }

auto Engine::getInstance(std::uint32_t width, std::uint32_t height, std::string_view name) -> Engine&
{
	LOG_INFO(logger, "Statically initialising engine {}; width: {}, height: {}", name, width, height);
	static auto engine = Engine{width, height, name};
	return engine;
}

auto Engine::run() -> void
{
	LOG_INFO(logger, "Running engine {}", name_);

	auto event = SDL_Event{};
	auto to_end = bool{};

	// Main event loop
	while (not to_end) {
		// Handle events from queue
		while (SDL_PollEvent(&event) != 0) {
			// If user closes window
			to_end = event.type == SDL_QUIT;

			// pause rendering if minimised or focus lost
			renderingIsPaused_ = event.type == SDL_WINDOWEVENT
			                   and (event.window.event == SDL_WINDOWEVENT_MINIMIZED
			                        or event.window.event == SDL_WINDOWEVENT_FOCUS_LOST);

			if (event.type == SDL_KEYDOWN) {
				LOG_INFO(logger, "Key pressed: {}", SDL_GetKeyName(event.key.keysym.sym));
			}
		}

		// Do not draw if minimised or backgrounded
		if (renderingIsPaused_) {
			std::this_thread::sleep_for(HUNDRED_MILLISECONDS);
			continue;
		}
		draw();
	}
}

Engine::Engine(std::uint32_t const width, std::uint32_t const height, std::string_view const name) :
    name_{name},
    frameIndex_{},
    renderingIsPaused_{},
    windowExtent_{.width = width, .height = height},
    window_{makeWindow()},
    bootstrapInstance_{makeBootstrapInstance()},
    instance_{context_, bootstrapInstance_.instance},
    debugMessenger_{instance_, bootstrapInstance_.debug_messenger},
    surface_{makeSDLSurface()},
    bootstrapGPU_{selectDevice()},
    bootstrapLogicalDevice_{vkb::DeviceBuilder{bootstrapGPU_}.build().value()},
    selectedGPU_{instance_, bootstrapGPU_.physical_device},
    logicalDevice_{selectedGPU_, bootstrapLogicalDevice_.device},
    swapchainData_{selectedGPU_, logicalDevice_, surface_, vk::PresentModeKHR::eFifo, windowExtent_},
    graphicsQueueFamily_{bootstrapLogicalDevice_.get_queue_index(vkb::QueueType::graphics).value()},
    graphicsQueue_{logicalDevice_, bootstrapLogicalDevice_.get_queue(vkb::QueueType::graphics).value()},
    frames_{FrameCommand::makeTripleBufferedFrames(logicalDevice_, graphicsQueueFamily_)}
{}

auto Engine::draw() -> void
{
	LOG_INFO(logger, "Drawing");

	// get the next image and image view.
	auto const wait_and_get_next_images = [this] -> std::tuple<unsigned, vk::Image&, vk::raii::ImageView&> {
		LOG_INFO(logger, "Waiting for and resetting previous fence");
		// wait for and reset the render fence for the current frame
		check_if_success(
		    logicalDevice_.waitForFences(*getCurrentFrame().renderFence_, true, NANOSECONDS_IN_A_SECOND.count()));
		logicalDevice_.resetFences(*getCurrentFrame().renderFence_);

		// acquire the swapchain fence, and then get the next image's index
		LOG_INFO(logger, "Acquiring next image");
		auto const [result, image_index] = swapchainData_.swapchain_.acquireNextImage(
		    NANOSECONDS_IN_A_SECOND.count(), getCurrentFrame().swapchainSemaphore_);
		check_if_success(result);
		return {image_index, swapchainData_.images_.at(image_index), swapchainData_.imageViews_.at(image_index)};
	};

	auto&& [next_image_index, next_image, next_image_view] = wait_and_get_next_images();

	// Reset the buffer and prime it for recording
	auto const buffer_after_reset_and_start_recording_once = [this] -> vk::raii::CommandBuffer& {
		auto&& cmd_buffer = getCurrentFrame().commandBuffer_;

		LOG_INFO(logger, "Resetting command buffer");
		check_if_success(cmd_buffer.reset());
		LOG_INFO(logger, "Restarting command buffer");
		check_if_success(cmd_buffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit}));
		return cmd_buffer;
	};
	auto&& cmd_buffer = buffer_after_reset_and_start_recording_once();

	// transition image into writeable mode
	transition_image(cmd_buffer, next_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

	// Clear the image
	auto const flash_value = vk::ClearColorValue{0.F, 0.F, std::abs(std::sin(frameIndex_ / 120.F)), 0.F};
	constexpr auto clear_range = vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
	                                                       .levelCount = vk::RemainingMipLevels,
	                                                       .layerCount = vk::RemainingArrayLayers};
	cmd_buffer.clearColorImage(next_image, vk::ImageLayout::eGeneral, flash_value, clear_range);

	// transition into presentable state
	transition_image(cmd_buffer, next_image, vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR);
	check_if_success(cmd_buffer.end());

	// Prepare to submit...
	auto const command_submit_info = vk::CommandBufferSubmitInfo{.commandBuffer = cmd_buffer};
	auto const wait_info = vk::SemaphoreSubmitInfo{.semaphore = getCurrentFrame().swapchainSemaphore_,
	                                               .value = 1,
	                                               .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput};
	auto const signal_info = vk::SemaphoreSubmitInfo{.semaphore = getCurrentFrame().renderSemaphore_,
	                                                 .value = 1,
	                                                 .stageMask = vk::PipelineStageFlagBits2::eAllGraphics};
	auto const submit_info = vk::SubmitInfo2{}
	                             .setSignalSemaphoreInfos(signal_info)
	                             .setWaitSemaphoreInfos(wait_info)
	                             .setCommandBufferInfos(command_submit_info);
	check_if_success(graphicsQueue_.submit2(submit_info, getCurrentFrame().renderFence_));

	// Prepare to present
	auto const present_info = vk::PresentInfoKHR{}
	                              .setSwapchains(*swapchainData_.swapchain_)
	                              .setWaitSemaphores(*getCurrentFrame().renderSemaphore_)
	                              .setImageIndices(next_image_index);
	check_if_success(graphicsQueue_.presentKHR(present_info));

	++frameIndex_;
}

auto Engine::makeWindow() const -> UniqueSDLWindow
{
	LOG_INFO(logger, "Initialising SDL");
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		LOG_ERROR(logger, "Failed to initialise SDL: {}", SDL_GetError());
		std::exit(EXIT_FAILURE);
	}
#pragma warning(push)
#pragma warning(disable : 4365)
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#endif
	LOG_INFO(logger, "Creating SDL window");
	if (auto* const window =
	        SDL_CreateWindow(name_.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                         windowExtent_.width,                      // NOLINT(*-narrowing-conversions)
	                         windowExtent_.height, SDL_WINDOW_VULKAN); // NOLINT(*-narrowing-conversions)
#pragma warning(pop)
#ifdef __clang__
#pragma clang diagnostic pop
#endif
	    window == nullptr) {
		LOG_ERROR(logger, "Failed to create SDL window: {}", SDL_GetError());
		std::exit(EXIT_FAILURE);
	} else {
		return UniqueSDLWindow{window, &SDL_DestroyWindow};
	}
}

auto Engine::makeBootstrapInstance() const -> vkb::Instance
{
	LOG_INFO(logger, "Creating bootstrap instance");

	static constexpr auto debug_callback_raw = PFN_vkDebugUtilsMessengerCallbackEXT{
	    [](VkDebugUtilsMessageSeverityFlagBitsEXT const severity, VkDebugUtilsMessageTypeFlagsEXT const type,
	       VkDebugUtilsMessengerCallbackDataEXT const* callback_data, [[maybe_unused]] void* data) -> vk::Bool32 {
		    static constexpr auto format = "[Vulkan: {}] : {}";

		    auto const debug_message_type = vk::DebugUtilsMessageTypeFlagsEXT{type};

		    switch (static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity)) {
		    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
			    LOG_TRACE_L1(logger, format, to_string(debug_message_type), callback_data->pMessage);
			    return vk::True;
		    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
			    LOG_INFO(logger, format, to_string(debug_message_type), callback_data->pMessage);
			    return vk::True;
		    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
			    LOG_WARNING(logger, format, to_string(debug_message_type), callback_data->pMessage);
			    return vk::True;
		    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
			    LOG_ERROR(logger, format, to_string(debug_message_type), callback_data->pMessage);
			    return vk::False;
		    default: std::unreachable();
		    }
	    }};

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
	constexpr auto USE_VALIDATION = true;
#else
	constexpr auto USE_VALIDATION = false;
#endif

	if (auto const instance_result = vkb::InstanceBuilder{}
	                                     .set_app_name(name_.data())
	                                     .enable_validation_layers(USE_VALIDATION)
	                                     .set_debug_callback(debug_callback_raw)
	                                     .require_api_version(1, 3, 0)
	                                     .build();
	    not instance_result) {
		LOG_ERROR(logger, "Failed to build boostrap instance: {}", instance_result.error().message());
		std::exit(EXIT_FAILURE);
	} else {
		return instance_result.value();
	}
}

auto Engine::makeSDLSurface() const -> vk::raii::SurfaceKHR
{
	auto raw_surface = VkSurfaceKHR{};
	if (SDL_Vulkan_CreateSurface(window_.get(), *instance_, &raw_surface) != SDL_TRUE) {
		LOG_ERROR(logger, "SDL surface was not created: {}", SDL_GetError());
		std::exit(EXIT_FAILURE);
	}
	return {instance_, raw_surface};
}

auto Engine::selectDevice() const -> vkb::PhysicalDevice
{
	LOG_INFO(logger, "Selecting GPU");

	constexpr auto vulkan_12_features =
	    vk::PhysicalDeviceVulkan12Features{}.setBufferDeviceAddress(vk::True).setDescriptorIndexing(vk::True);
	constexpr auto vulkan_13_features =
	    vk::PhysicalDeviceVulkan13Features{}.setDynamicRendering(vk::True).setSynchronization2(vk::True);

	auto const selector = vkb::PhysicalDeviceSelector{bootstrapInstance_}
	                          .set_minimum_version(1, 3)
	                          .set_required_features_13(vulkan_13_features)
	                          .set_required_features_12(vulkan_12_features)
	                          .set_surface(*surface_);

	if (auto maybe_device = selector.select(); not maybe_device.has_value()) {
		LOG_ERROR(logger, "Could not find a physical device:\n{}", maybe_device.detailed_failure_reasons());
		std::exit(EXIT_FAILURE);
	} else {
		LOG_INFO(logger, "Selected GPU: {}", maybe_device.value().name);
		return maybe_device.value();
	}
}

auto Engine::getCurrentFrame() -> FrameCommand& { return frames_.at(frameIndex_ % FrameCommand::FRAME_OVERLAP); }
} // namespace caldera
