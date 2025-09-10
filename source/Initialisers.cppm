module Caldera:Initialisers;

import std;
import vulkan_hpp;

namespace caldera::init
{
constexpr auto makeImageCreateInfo(vk::Format const& image_format,
                                   vk::Extent3D const& extent,
                                   vk::ImageUsageFlags const& usage_flags) -> vk::ImageCreateInfo
{
	return {.imageType = vk::ImageType::e2D,
	        .format = image_format,
	        .extent = extent,
	        .mipLevels = 1U,
	        .arrayLayers = 1U,
	        .samples = vk::SampleCountFlagBits::e1,
	        .tiling = vk::ImageTiling::eOptimal,
	        .usage = usage_flags};
}

constexpr auto makeImageViewCreateInfo(vk::Format const& format,
                                       vk::Image const& image,
                                       vk::ImageAspectFlags const& aspect_flags) -> vk::ImageViewCreateInfo
{
	return {.image = image,
	        .viewType = vk::ImageViewType::e2D,
	        .format = format,
	        .subresourceRange = {.aspectMask = aspect_flags, .levelCount = 1U, .layerCount = 1U}};
}
} // namespace caldera::init
