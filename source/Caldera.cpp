module;

#include <quill/LogMacros.h>
#include <SDL2/SDL.h>
#include <VkBootstrap.h>

export module Caldera;

import :Types;
import vulkan;

export namespace caldera
{
struct Engine
{
	~Engine();
	Engine(Engine const& other) = delete;
	Engine(Engine&& other) noexcept = delete;
	auto operator=(Engine const& other) -> Engine& = delete;
	auto operator=(Engine&& other) noexcept -> Engine& = delete;

	static auto getInstance(std::uint32_t width, std::uint32_t height, std::string_view name) -> Engine&;
	auto run() -> void;

private:
	explicit Engine(std::uint32_t width, std::uint32_t height, std::string_view name);

	auto draw() -> void;

	// Basic engine details
	std::string name_;
	std::size_t frameIndex_;
	bool renderingIsPaused_;
	vk::Extent2D windowExtent_;

	// Window to render to
	using UniqueSDLWindow = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
	auto makeWindow() const -> UniqueSDLWindow;
	UniqueSDLWindow window_;

	// Vulkan basics: RAII context, Vulkan instance, debug messenger
	auto makeBootstrapInstance() const -> vkb::Instance;
	vk::raii::Context context_;
	vkb::Instance bootstrapInstance_;
	vk::raii::Instance instance_;
	vk::raii::DebugUtilsMessengerEXT debugMessenger_;

	// SDL surface to render to
	auto makeSDLSurface() const -> vk::raii::SurfaceKHR;
	vk::raii::SurfaceKHR surface_;

	// Device-related; requires feature selection
	auto selectDevice() const -> vkb::PhysicalDevice;
	vkb::PhysicalDevice bootstrapGPU_;
	vkb::Device bootstrapLogicalDevice_;
	vk::raii::PhysicalDevice selectedGPU_;
	vk::raii::Device logicalDevice_;

	// Swapchain and frame data
	auto getCurrentFrame() -> FrameCommand&;
	SwapchainData swapchainData_;
	std::uint32_t graphicsQueueFamily_;
	vk::raii::Queue graphicsQueue_;
	FrameCommand::TripleBufferedFrames frames_;
};
} // namespace caldera
