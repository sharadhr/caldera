module;

#include <quill/LogMacros.h>
#include <quill/std/Vector.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <VkBootstrap.h>

module Caldera;

import :Util;
import vk_mem_alloc;
import vulkan;

using namespace std::literals;

constexpr auto HUNDRED_MILLISECONDS = 100ms;

namespace caldera
{
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
			renderingPaused_ = event.type == SDL_WINDOWEVENT
			                   and (event.window.event == SDL_WINDOWEVENT_MINIMIZED
			                        or event.window.event == SDL_WINDOWEVENT_FOCUS_LOST);

			if (event.type == SDL_KEYDOWN) {
				LOG_INFO(logger, "Key pressed: {}", SDL_GetKeyName(event.key.keysym.sym));
			}
		}

		// Do not draw if minimised or backgrounded
		if (renderingPaused_) {
			std::this_thread::sleep_for(HUNDRED_MILLISECONDS);
			continue;
		}
		draw();
	}
}

Engine::Engine(std::uint32_t const width, std::uint32_t const height, std::string_view const name) :
    name_{name},
    frameIndex_{},
    renderingPaused_{},
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
    swapchainData_{selectedGPU_, logicalDevice_, surface_, windowExtent_},
    graphicsQueueFamily_{bootstrapLogicalDevice_.get_queue_index(vkb::QueueType::graphics).value()},
    graphicsQueue_{logicalDevice_, bootstrapLogicalDevice_.get_queue(vkb::QueueType::graphics).value()},
    frames_{FrameCommand::makeTripleBufferedFrames(logicalDevice_, graphicsQueueFamily_)}
{}

auto Engine::draw() -> void { LOG_INFO(logger, "Drawing"); }

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
			    return vk::False;
		    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
			    LOG_ERROR(logger, format, to_string(debug_message_type), callback_data->pMessage);
			    return vk::False;
		    default: std::unreachable();
		    }
	    }};

	if (auto const instance_result = vkb::InstanceBuilder{}
	                                     .set_app_name(name_.data())
	                                     .request_validation_layers(USE_VALIDATION)
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

	constexpr auto vulkan_13_features =
	    vk::PhysicalDeviceVulkan13Features{}.setDynamicRendering(vk::True).setSynchronization2(vk::True);
	constexpr auto vulkan_12_features =
	    vk::PhysicalDeviceVulkan12Features{}.setBufferDeviceAddress(vk::True).setDescriptorIndexing(vk::True);

	auto const selector = vkb::PhysicalDeviceSelector{bootstrapInstance_}
	                          .set_minimum_version(1, 3)
	                          .set_required_features_13(vulkan_13_features)
	                          .set_required_features_12(vulkan_12_features)
	                          .set_surface(*surface_);

	if (auto maybe_device = selector.select(); not maybe_device.has_value()) {
		LOG_ERROR(logger, "Could not find a physical device:\n{}", maybe_device.detailed_failure_reasons());
		std::exit(EXIT_FAILURE);
	} else {
		return maybe_device.value();
	}
}
} // namespace caldera
