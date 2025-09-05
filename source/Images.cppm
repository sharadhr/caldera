module;

export module Caldera.Images;

import std;
import vulkan_hpp;

export namespace caldera::util
{
auto transitionImage(vk::raii::CommandBuffer const& command,
                     vk::Image const& image,
                     vk::ImageLayout current_layout,
                     vk::ImageLayout new_layout)
{
	auto const aspect_mask = new_layout == vk::ImageLayout::eDepthAttachmentOptimal ? vk::ImageAspectFlagBits::eDepth
	                                                                                : vk::ImageAspectFlagBits::eColor;
	auto const subresource_range = vk::ImageSubresourceRange{.aspectMask = aspect_mask,
	                                                         .levelCount = vk::RemainingMipLevels,
	                                                         .layerCount = vk::RemainingArrayLayers};
	auto const image_barrier =
		vk::ImageMemoryBarrier2{.srcStageMask = vk::PipelineStageFlagBits2::eAllCommands,
	                          .srcAccessMask = vk::AccessFlagBits2::eMemoryWrite,
	                          .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
	                          .dstAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
	                          .oldLayout = current_layout,
	                          .newLayout = new_layout,
	                          .image = image,
	                          .subresourceRange = subresource_range};

	auto const dep_info = vk::DependencyInfo{}.setImageMemoryBarriers(image_barrier);
	command.pipelineBarrier2(dep_info);
}
} // namespace caldera::util
