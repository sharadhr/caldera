module;

#include <quill/LogMacros.h>
#include <SDL2/SDL.h>
#include <VkBootstrap.h>

export module Caldera;

export import :Util;
import vulkan;

export namespace caldera
{
struct Engine
{
	~Engine() = default;
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
	std::size_t frameIndex_{};
	bool renderingPaused_{};
	vk::Extent2D windowExtent_;

	using UniqueSDLWindow = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
	auto makeWindow() const -> UniqueSDLWindow;
	UniqueSDLWindow window_;

	// Vulkan basics: RAII context, Vulkan instance, debug messenger
	auto makeBootstrapInstance() const -> vkb::Instance;
	vk::raii::Context context_;
	vkb::Instance vkbsInstance_;
	vk::raii::Instance instance_;
	vk::raii::DebugUtilsMessengerEXT debugMessenger_;

	// SDL surface to render to
	auto makeSDLSurface() const -> vk::raii::SurfaceKHR;
	vk::raii::SurfaceKHR surface_;

	// Device-related; requires feature selection
	auto selectDevice() const -> vkb::PhysicalDevice;
	vkb::PhysicalDevice vkbsSelectedPhysicalDevice_;
	vk::raii::PhysicalDevice selectedPhysicalDevice_;
	vk::raii::Device device_;
};
} // namespace caldera
