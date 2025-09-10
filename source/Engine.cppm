module;

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <spdlog/spdlog.h>
#include <VkBootstrap.h>

export module Caldera:Engine;

import :Images;
import :Initialisers;
import :Types;

import std;
import vk_mem_alloc_hpp;
import vulkan_hpp;

export namespace caldera
{
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
	auto draw() -> void;
	auto drawBackground(vk::raii::CommandBuffer const& command_buffer) const -> void;
	auto run() -> void;

private:
	using UniqueSDLWindow = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;

	std::string name_;
	auto makeWindow() const -> UniqueSDLWindow;
	auto buildBootstrapInstance() const -> vkb::Instance;
	auto buildBootstrapDevice() const -> vkb::PhysicalDevice;
	auto makeSurface() -> vk::raii::SurfaceKHR;
	auto buildBootstrapSwapchain() const -> vkb::Swapchain;
	auto makeSwapchainImageViews() -> std::vector<vk::raii::ImageView>;

	std::size_t frameIndex_{};
	bool pauseRendering_{};
	vk::Extent2D windowExtent_;
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
	vma::Allocator allocator_;
	AllocatedImage drawImage_;
	vk::Extent2D drawExtent_;
};

} // namespace caldera

using namespace std::literals;

constexpr auto ONE_SECOND = 1s;
constexpr auto HUNDRED_MILLISECONDS = 100ms;
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
		default: std::unreachable();
		}

		return vk::False;
	}};

namespace caldera
{
Engine::Engine(std::uint32_t width, std::uint32_t height, std::string_view const name) :
	name_{name},
	windowExtent_{.width = width, .height = height},
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
	frames_{FrameCommand::makeFrameCommands(logicalDevice_, graphicsQueueFamily_)},
	allocator_{vma::createAllocator({.flags = vma::AllocatorCreateFlagBits::eBufferDeviceAddress,
                                   .physicalDevice = physicalDevice_,
                                   .device = logicalDevice_,
                                   .instance = instance_,
                                   .vulkanApiVersion = vk::ApiVersion13})
               .value},
	drawImage_{{windowExtent_.width, windowExtent_.height, 1U},
             vk::Format::eR16G16B16A16Sfloat,
             logicalDevice_,
             allocator_,
             vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst
               | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment}
{}

auto Engine::getCurrentFrame() -> FrameCommand& { return frames_.at(frameIndex_ % FrameCommand::FRAME_OVERLAP); }

auto Engine::draw() -> void
{
#pragma warning(push)
#pragma warning(disable : 4365)
	static constexpr auto nanoseconds_in_a_second = NANOSECONDS_IN_ONE_SECOND.count();
#pragma warning(pop)

	// wait for and reset last frame
	checkResult(logicalDevice_.waitForFences(*getCurrentFrame().renderFence_, vk::True, nanoseconds_in_a_second));

	logicalDevice_.resetFences(*getCurrentFrame().renderFence_);

	// request image from swapchain
	auto [result, swapchain_image_index] =
		logicalDevice_.acquireNextImage2KHR({.swapchain = swapchain_,
	                                       .timeout = nanoseconds_in_a_second,
	                                       .semaphore = *getCurrentFrame().swapchainSemaphore_,
	                                       .deviceMask = 1U});
	checkResult(result);

	auto const& draw_image = drawImage_.imageAndMemory_.first;
	auto const& swapchain_image = swapchainImages_[swapchain_image_index];

	auto const& command_buffer = getCurrentFrame().mainCommandBuffer_;
	command_buffer.reset();

	drawExtent_ = {.width = drawImage_.extent_.width, .height = drawImage_.extent_.height};

	// *** Start recording into the command buffer
	command_buffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

	// transition draw image from undefined to general
	util::transitionImage(command_buffer, draw_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

	drawBackground(command_buffer);

	// transition swapchain image and draw image
	util::transitionImage(command_buffer, draw_image, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);
	util::transitionImage(command_buffer, swapchain_image, vk::ImageLayout::eUndefined,
	                      vk::ImageLayout::eTransferDstOptimal);

	// copy the image
	util::copyImageToImage(command_buffer, draw_image, swapchain_image, drawExtent_, swapchainExtent_);

	// transition from general to presentable
	util::transitionImage(command_buffer, swapchain_image, vk::ImageLayout::eTransferDstOptimal,
	                      vk::ImageLayout::ePresentSrcKHR);

	command_buffer.end();

	// *** Submit the command buffer ***
	auto const command_submit_info = vk::CommandBufferSubmitInfo{.commandBuffer = command_buffer};

	auto const wait_info = vk::SemaphoreSubmitInfo{.semaphore = getCurrentFrame().swapchainSemaphore_,
	                                               .value = 1,
	                                               .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput};
	auto const signal_info = vk::SemaphoreSubmitInfo{.semaphore = getCurrentFrame().renderSemaphore_,
	                                                 .value = 1,
	                                                 .stageMask = vk::PipelineStageFlagBits2::eAllGraphics};

	auto const submit_info = vk::SubmitInfo2{}
	                           .setWaitSemaphoreInfos(wait_info)
	                           .setCommandBufferInfos(command_submit_info)
	                           .setSignalSemaphoreInfos(signal_info);

	// submit command buffer to queue blocking against render fence
	graphicsQueue_.submit2(submit_info, getCurrentFrame().renderFence_);

	// present the frame
	checkResult(graphicsQueue_.presentKHR(vk::PresentInfoKHR{}
	                                        .setWaitSemaphores(*getCurrentFrame().renderSemaphore_)
	                                        .setSwapchains(*swapchain_)
	                                        .setImageIndices(swapchain_image_index)));
	++frameIndex_;
}

auto Engine::drawBackground(vk::raii::CommandBuffer const& command_buffer) const -> void
{
	// clear-colour from the frame index; flash with 120-frame period
	auto const flash = std::abs(std::sin(frameIndex_ / 120.F));
	auto const clear_value = vk::ClearColorValue{std::array{0.F, 0.F, flash, 1.F}};
	constexpr auto clear_range = vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
	                                                       .levelCount = vk::RemainingMipLevels,
	                                                       .layerCount = vk::RemainingArrayLayers};
	// clear the image
	command_buffer.clearColorImage(drawImage_.imageAndMemory_.first, vk::ImageLayout::eGeneral, clear_value, clear_range);
}

auto Engine::run() -> void
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
				SPDLOG_INFO("Key pressed: {}"sv, SDL_GetKeyName(event.key.keysym.sym));
			}

			if (pauseRendering_) {
				using namespace std::literals;

				// std::this_thread::sleep_for(HUNDRED_MILLISECONDS);
			}
		}
		draw();
	}
	graphicsQueue_.waitIdle();
}

auto Engine::makeWindow() const -> UniqueSDLWindow
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		spdlog::error("Failed to initialise SDL: {}"sv, SDL_GetError());
		std::exit(EXIT_FAILURE);
	}

#pragma warning(push)
#pragma warning(disable : 4365)
	if (auto* const window = SDL_CreateWindow(name_.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                                          windowExtent_.width,                      // NOLINT(*-narrowing-conversions)
	                                          windowExtent_.height, SDL_WINDOW_VULKAN); // NOLINT(*-narrowing-conversions)
#pragma warning(pop)
	    window == nullptr) {
		spdlog::error("Failed to create SDL Window: {}"sv, SDL_GetError());
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
	        .set_desired_extent(windowExtent_.width, windowExtent_.height)
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
} // namespace caldera
