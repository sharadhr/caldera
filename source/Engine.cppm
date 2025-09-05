module;

#include "vulkan/vulkan_raii.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <VkBootstrap.h>
#include <spdlog/spdlog.h>

export module Caldera.Engine;

import Caldera.Images;
import Caldera.Initialisers;
import Caldera.Types;
import std;
import vulkan_hpp;

export namespace caldera
{
struct FrameCommand
{
	static constexpr auto FRAME_OVERLAP = 3U;
	using TripleBuffered = std::array<FrameCommand, FRAME_OVERLAP>;

	explicit FrameCommand(vk::raii::Device const& logical_device,
	                      std::uint32_t queue_family_index,
	                      vk::CommandPoolCreateFlagBits flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	static auto
	makeFrameCommands(vk::raii::Device const& device,
	                  std::uint32_t queue_family_index,
	                  vk::CommandPoolCreateFlagBits flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		-> TripleBuffered;

	vk::raii::CommandPool commandPool_;
	vk::raii::CommandBuffer mainCommandBuffer_;
	vk::raii::Semaphore swapchainSemaphore_;
	vk::raii::Semaphore renderSemaphore_;
	vk::raii::Fence renderFence_;
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
	auto getCurrentFrame() -> FrameCommand&;
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

	auto initSyncStructures() -> void;

	std::size_t frameIndex_{};
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
	vk::Format swapchainImageFormat_{vk::Format::eB8G8R8A8Unorm};
	std::vector<vk::Image> swapchainImages_;
	std::vector<vk::raii::ImageView> swapchainImageViews_;
	vk::Extent2D swapchainExtent_;
	std::uint32_t graphicsQueueFamily_;
	vk::raii::Queue graphicsQueue_;
	FrameCommand::TripleBuffered frames_;
};

} // namespace caldera

module :private;

namespace
{
using namespace std::literals;

constexpr auto ONE_SECOND = 1s;
constexpr auto NANOSECONDS_IN_ONE_SECOND = std::chrono::nanoseconds{ONE_SECOND};
constexpr auto debugCallbackRaw =
	PFN_vkDebugUtilsMessengerCallbackEXT{[](VkDebugUtilsMessageSeverityFlagBitsEXT const severity,
                                          VkDebugUtilsMessageTypeFlagsEXT const type,
                                          VkDebugUtilsMessengerCallbackDataEXT const* callback_data,
                                          [[maybe_unused]] void* data) {
		static constexpr auto format = "[Vulkan: {}] : {}"sv;

		switch (static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity)) {
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
#if defined(DEBUG) || defined(_DEBUG) || !defined(_NDEBUG)
			spdlog::trace(format, to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(type)), callback_data->pMessage);
#endif
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
#if defined(DEBUG) || defined(_DEBUG) || !defined(_NDEBUG)
			spdlog::info(format, vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(type)),
		               callback_data->pMessage);
#endif
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
			spdlog::warn(format, vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(type)),
		               callback_data->pMessage);
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
			spdlog::error(format, vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(type)),
		                callback_data->pMessage);
			break;
		default: break;
		}

		return vk::False;
	}};
} // namespace

namespace caldera
{
FrameCommand::FrameCommand(vk::raii::Device const& logical_device,
                           std::uint32_t queue_family_index,
                           vk::CommandPoolCreateFlagBits flags) :
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

auto FrameCommand::makeFrameCommands(vk::raii::Device const& device,
                                     std::uint32_t queue_family_index,
                                     vk::CommandPoolCreateFlagBits flags) -> TripleBuffered
{
	// there's probably a nice template way to produce an array from a range
	return TripleBuffered{FrameCommand(device, queue_family_index, flags),
	                      FrameCommand(device, queue_family_index, flags),
	                      FrameCommand(device, queue_family_index, flags)};
}

Engine::Engine(std::uint32_t width, std::uint32_t height, std::string_view const name) :
	name_{name},
	extent_{.width = width, .height = height},
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
	swapchainImages_(swapchain_.getImages()),
	swapchainImageViews_(makeSwapchainImageViews()),
	swapchainExtent_{.width = bootstrapSwapchain_.extent.width, .height = bootstrapSwapchain_.extent.height},
	graphicsQueueFamily_{bootstrapLogicalDevice_.get_queue_index(vkb::QueueType::graphics).value()},
	graphicsQueue_{logicalDevice_, bootstrapLogicalDevice_.get_queue(vkb::QueueType::graphics).value()},
	frames_{FrameCommand::makeFrameCommands(logicalDevice_, graphicsQueueFamily_)}
{
	initSyncStructures();
}

auto Engine::getInstance(std::uint32_t width, std::uint32_t height, std::string_view const name) -> Engine&
{
	static auto engine = Engine{width, height, name};
	return engine;
}

auto Engine::getCurrentFrame() -> FrameCommand& { return frames_[frameIndex_ % FrameCommand::FRAME_OVERLAP]; }

void Engine::draw()
{
	// wait for and reset last frame
	vkCheck(logicalDevice_.waitForFences(*getCurrentFrame().renderFence_, vk::True, NANOSECONDS_IN_ONE_SECOND.count()));
	logicalDevice_.resetFences(*getCurrentFrame().renderFence_);

	// request image from swapchain
	auto [result, swapchain_image_index] =
		logicalDevice_.acquireNextImage2KHR({.swapchain = *swapchain_,
	                                       .timeout = NANOSECONDS_IN_ONE_SECOND.count(),
	                                       .semaphore = *getCurrentFrame().swapchainSemaphore_,
	                                       .deviceMask = 1U});
	vkCheck(result);
	auto const& swapchain_image = swapchainImages_[swapchain_image_index];

	//
	auto const& command_buffer = getCurrentFrame().mainCommandBuffer_;
	command_buffer.reset();

	// start recording command buffer
	command_buffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

	// transition from undefined to general
	util::transitionImage(command_buffer, swapchain_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

	// clear-colour from the frame index; flash with 120-frame period
	auto const flash = std::abs(std::sin(frameIndex_ / 120.F));
	auto const clear_value = vk::ClearColorValue{std::array{0.F, 0.F, flash, 1.F}};
	constexpr auto clear_range = vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
	                                                       .levelCount = vk::RemainingMipLevels,
	                                                       .layerCount = vk::RemainingArrayLayers};

	// clear the image
	command_buffer.clearColorImage(swapchain_image, vk::ImageLayout::eGeneral, clear_value, clear_range);

	// transition from general to presentable
	util::transitionImage(command_buffer, swapchain_image, vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR);

	command_buffer.end();

	auto const command_submit_info = vk::CommandBufferSubmitInfo{.commandBuffer = command_buffer};

	auto const wait_info = vk::SemaphoreSubmitInfo{.semaphore = getCurrentFrame().swapchainSemaphore_,
	                                               .value = 1,
	                                               .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput};
	auto const signal_info = vk::SemaphoreSubmitInfo{
		.semaphore = getCurrentFrame().renderSemaphore_, .value = 1, .stageMask = vk::PipelineStageFlagBits2::eAllGraphics};

	auto const submit_info = vk::SubmitInfo2{}
	                           .setWaitSemaphoreInfos(wait_info)
	                           .setCommandBufferInfos(command_submit_info)
	                           .setSignalSemaphoreInfos(signal_info);

	// submit command buffer to queue blocking against render fence
	graphicsQueue_.submit2(submit_info, getCurrentFrame().renderFence_);

	// present the frame
	vkCheck(graphicsQueue_.presentKHR(vk::PresentInfoKHR{}
															.setWaitSemaphores(*getCurrentFrame().renderSemaphore_)
															.setSwapchains(*swapchain_)
															.setImageIndices(swapchain_image_index)));
	++frameIndex_;
}

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
			draw();
		}
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

auto Engine::initSyncStructures() -> void {}
} // namespace caldera
