module;

#include "vulkan/vulkan_raii.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <VkBootstrap.h>
#include <spdlog/spdlog.h>

export module Caldera.Engine;

import std;
import vulkan_hpp;

export namespace caldera
{
struct Frame
{
	static constexpr auto FRAME_OVERLAP = 2U;
	vk::raii::CommandPool commandPool_;
	vk::raii::CommandBuffer mainCommandBuffer_;
};
struct Engine
{
	explicit Engine(std::uint32_t width, std::uint32_t height, std::string_view name);
	~Engine() = default;
	Engine(Engine const& other) = delete;
	Engine(Engine&& other) noexcept = delete;
	auto operator=(Engine const& other) -> Engine& = delete;
	auto operator=(Engine&& other) noexcept -> Engine& = delete;

	static auto getInstance(std::uint32_t width, std::uint32_t height, std::string_view name) -> Engine&;
	void draw();
	void run();

private:
	using UniqueSDLWindow = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;

	std::string name_;
	auto makeWindow() const -> UniqueSDLWindow;
	auto buildBootstrapInstance() const -> vkb::Instance;
	auto buildBootstrapDevice() const -> vkb::PhysicalDevice;
	auto makeSurface() -> vk::raii::SurfaceKHR;
	auto buildBootstrapSwapchain() const -> vkb::Swapchain;
	auto makeSwapchainImageViews() -> std::vector<vk::raii::ImageView>;

	auto initCommands() -> void;
	auto initSyncStructures() -> void;

	std::size_t frame_{};
	bool pauseRendering_{};
	vk::Extent2D extent_;
	UniqueSDLWindow window_;
	vk::raii::Context context_;
	vkb::Instance bootstrapInstance_;
	vk::raii::Instance instance_;
	vk::raii::DebugUtilsMessengerEXT debugMessenger_;
	VkSurfaceKHR rawSurface_{};
	vk::raii::SurfaceKHR surface_;
	vkb::PhysicalDevice bootstrapPhysicalDevice_;
	vkb::Device bootstrapLogicalDevice_;
	vk::raii::PhysicalDevice physicalDevice_;
	vk::raii::Device logicalDevice_;
	vkb::Swapchain bootstrapSwapchain_;
	vk::raii::SwapchainKHR swapchain_;
	vk::Format swapchainImageFormat_;
	std::vector<vk::Image> swapchainImages_;
	std::vector<vk::raii::ImageView> swapchainImageViews_;
	vk::Extent2D swapchainExtent_;
	// std::array<Frame, Frame::FRAME_OVERLAP> frames_;
	// vk::raii::Queue graphicsQueue_;
};

} // namespace caldera

module :private;

using namespace std::literals;

namespace
{
constexpr auto debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT const severity,
                   vk::DebugUtilsMessageTypeFlagsEXT const type,
                   vk::DebugUtilsMessengerCallbackDataEXT const* callback_data,
                   [[maybe_unused]] void* data) -> vk::Bool32
{
	static constexpr auto format = "[Vulkan: {}] : {}"sv;

	switch (severity) {
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
#if defined(DEBUG) || defined(_DEBUG) || !defined(_NDEBUG)
		SPDLOG_TRACE(format, type., callback_data.pMessage);
#endif
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
#if defined(DEBUG) || defined(_DEBUG) || !defined(_NDEBUG)
		SPDLOG_INFO(format, to_string(type), callback_data->pMessage);
#endif
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
		spdlog::warn(format, to_string(type), callback_data->pMessage);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
		spdlog::error(format, vk::to_string(type), callback_data->pMessage);
		break;
	}

	return vk::False;
}

constexpr PFN_vkDebugUtilsMessengerCallbackEXT debugCallbackRaw = [](VkDebugUtilsMessageSeverityFlagBitsEXT const severity,
                                                           VkDebugUtilsMessageTypeFlagsEXT const type,
                                                           VkDebugUtilsMessengerCallbackDataEXT const* callback_data,
                                                           [[maybe_unused]] void* data) {
	return debugCallback(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity),
	                     vk::DebugUtilsMessageTypeFlagsEXT(type),
	                     reinterpret_cast<vk::DebugUtilsMessengerCallbackDataEXT const*>(callback_data), data);
};
} // namespace

namespace caldera
{
Engine::Engine(std::uint32_t width, std::uint32_t height, std::string_view const name) :
	name_{name},
	extent_{width, height},
	window_{makeWindow()},
	bootstrapInstance_{buildBootstrapInstance()},
	instance_{context_, bootstrapInstance_.instance},
	debugMessenger_{instance_, bootstrapInstance_.debug_messenger},
	surface_{makeSurface()},
	bootstrapPhysicalDevice_{buildBootstrapDevice()},
	bootstrapLogicalDevice_{vkb::DeviceBuilder{bootstrapPhysicalDevice_}.build().value()},
	physicalDevice_{instance_, bootstrapPhysicalDevice_.physical_device},
	logicalDevice_{physicalDevice_, bootstrapLogicalDevice_.device},
	bootstrapSwapchain_{buildBootstrapSwapchain()},
	swapchain_{logicalDevice_, bootstrapSwapchain_.swapchain},
	swapchainImageFormat_{vk::Format::eB8G8R8A8Unorm},
	swapchainImages_(swapchain_.getImages()),
	swapchainImageViews_(makeSwapchainImageViews()),
	swapchainExtent_{.width = bootstrapSwapchain_.extent.width, .height = bootstrapSwapchain_.extent.height}
{
	initCommands();
	initSyncStructures();
}

auto Engine::getInstance(std::uint32_t width, std::uint32_t height, std::string_view const name) -> Engine&
{
	static auto engine = Engine{width, height, name};
	return engine;
}

void Engine::draw() {}

void Engine::run()
{
	auto event = SDL_Event{};
	auto toEnd = bool{};

	// Main event loop
	while (not toEnd) {
		// Handle events from queue
		while (SDL_PollEvent(&event) != 0) {

			// If user closes window
			toEnd = event.type == SDL_QUIT;

			// pause rendering if minimised
			pauseRendering_ = event.type == SDL_WINDOWEVENT and event.window.event == SDL_WINDOWEVENT_MINIMIZED;

			if (event.type == SDL_KEYDOWN) {
				SPDLOG_INFO("Key pressed: {}", SDL_GetKeyName(event.key.keysym.sym));
			}

			if (pauseRendering_) {
				using namespace std::literals;

				std::this_thread::sleep_for(100ms);
			}
		}

		draw();
	}
}

auto Engine::makeWindow() const -> UniqueSDLWindow
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		spdlog::error("Failed to initialise SDL: {}", SDL_GetError());
		std::exit(EXIT_FAILURE);
	}

	if (auto const window = SDL_CreateWindow(name_.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                                         extent_.width, extent_.height, SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);
	    window == nullptr) {
		spdlog::error("Failed to create SDL Window: {}", SDL_GetError());
		std::exit(EXIT_FAILURE);
	} else {
		return UniqueSDLWindow{window, &SDL_DestroyWindow};
	}
}

auto Engine::buildBootstrapInstance() const -> vkb::Instance
{
	if (auto const instance_result = vkb::InstanceBuilder{}
	                                   .set_app_name(name_.data())
#if defined(DEBUG) || defined(_DEBUG) || !defined(_NDEBUG)
	                                   .request_validation_layers(true)
#endif
	                                   .set_debug_callback(debugCallbackRaw)
	                                   .require_api_version(1, 3, 0)
	                                   .build();
	    !instance_result) {
		spdlog::error("Failed to build boostrap instance: {}", instance_result.error().message());
		std::exit(EXIT_FAILURE);
	} else {
		return instance_result.value();
	}
}

auto Engine::buildBootstrapDevice() const -> vkb::PhysicalDevice
{
	constexpr auto features_1_2 =
		vk::PhysicalDeviceVulkan12Features{.descriptorIndexing = vk::True, .bufferDeviceAddress = vk::True};
	constexpr auto features_1_3 =
		vk::PhysicalDeviceVulkan13Features{.synchronization2 = vk::True, .dynamicRendering = vk::True};

	if (auto const device = vkb::PhysicalDeviceSelector{bootstrapInstance_, *surface_}
	                          .set_minimum_version(1, 3)
	                          .set_required_features_13(features_1_3)
	                          .set_required_features_12(features_1_2)
	                          .select();
	    !device) {
		spdlog::error("Failed to build bootstrap device: {}", device.error().message());
		std::exit(EXIT_FAILURE);
	} else {
		return device.value();
	}
}

auto Engine::makeSurface() -> vk::raii::SurfaceKHR
{
	if (SDL_Vulkan_CreateSurface(window_.get(), *instance_, &rawSurface_) != SDL_TRUE) {
		spdlog::error("Failed to create SDL surface: {}", SDL_GetError());
		std::exit(EXIT_FAILURE);
	}

	return vk::raii::SurfaceKHR{instance_, rawSurface_};
}

auto Engine::buildBootstrapSwapchain() const -> vkb::Swapchain
{
	auto const surface_format = vk::SurfaceFormatKHR{swapchainImageFormat_, vk::ColorSpaceKHR::eSrgbNonlinear};

	if (auto const maybe_swapchain =
	      vkb::SwapchainBuilder{*physicalDevice_, *logicalDevice_, *surface_}
	        .set_desired_format(surface_format)
	        .set_desired_present_mode(static_cast<VkPresentModeKHR>(vk::PresentModeKHR::eImmediate))
	        .set_desired_extent(extent_.width, extent_.height)
	        .add_image_usage_flags(static_cast<VkImageUsageFlags>(vk::ImageUsageFlagBits::eTransferDst))
	        .build();
	    !maybe_swapchain) {
		spdlog::error("Failed to create bootstrap swapchain: {}", maybe_swapchain.error().message());
		std::exit(EXIT_FAILURE);
	} else {
		return maybe_swapchain.value();
	}
}

auto Engine::makeSwapchainImageViews() -> std::vector<vk::raii::ImageView>
{
	return bootstrapSwapchain_.get_image_views().value()
	       | std::views::transform([&](auto&& image_view) { return vk::raii::ImageView{logicalDevice_, image_view}; })
	       | std::ranges::to<decltype(swapchainImageViews_)>();
}

auto Engine::initCommands() -> void {}

auto Engine::initSyncStructures() -> void {}
} // namespace caldera