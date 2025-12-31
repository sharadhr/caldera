module;

#include <quill/LogMacros.h>

module Caldera:Images;

import vulkan;
import QuillStatic;

namespace caldera
{
auto transitionImage(vk::raii::CommandBuffer const& cmd_buffer,
                     vk::Image const& image,
                     vk::ImageLayout const old_layout,
                     vk::ImageLayout const new_layout) -> void
{
	LOG_INFO(logger, "Transitioning image from {} to {}", vk::to_string(old_layout), vk::to_string(new_layout));
	// Always colour, unless depth attachment specified
	auto const aspect_mask = new_layout == vk::ImageLayout::eDepthAttachmentOptimal ? vk::ImageAspectFlagBits::eDepth
	                                                                                : vk::ImageAspectFlagBits::eColor;

	// Target the entire image
	auto const subresource_range = vk::ImageSubresourceRange{.aspectMask = aspect_mask,
	                                                         .levelCount = vk::RemainingMipLevels,
	                                                         .layerCount = vk::RemainingArrayLayers};
	auto const image_barrier =
	    vk::ImageMemoryBarrier2{.srcStageMask = vk::PipelineStageFlagBits2::eAllCommands,
	                            .srcAccessMask = vk::AccessFlagBits2::eMemoryWrite,
	                            .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
	                            .dstAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
	                            .oldLayout = old_layout,
	                            .newLayout = new_layout,
	                            .image = image,
	                            .subresourceRange = subresource_range};
	auto const dep_info = vk::DependencyInfo{}.setImageMemoryBarriers(image_barrier);

	cmd_buffer.pipelineBarrier2(dep_info);
}

auto blitImage(vk::raii::CommandBuffer const& cmd_buffer,
               vk::Image const& source,
               vk::Extent2D const source_extent,
               vk::Image const& destination,
               vk::Extent2D const destination_extent)
{
	auto const blit_region =
	    vk::ImageBlit2{.srcSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1},
	                   .srcOffsets = {{{{},
	                                    {.x = static_cast<int32_t>(source_extent.width),
	                                     .y = static_cast<int32_t>(source_extent.height),
	                                     .z = 1U}}}},
	                   .dstSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1},
	                   .dstOffsets = {{{{},
	                                    {.x = static_cast<int32_t>(destination_extent.width),
	                                     .y = static_cast<int32_t>(destination_extent.height),
	                                     .z = 1U}}}}};
	auto const blit_info = vk::BlitImageInfo2{.srcImage = source,
	                                          .srcImageLayout = vk::ImageLayout::eTransferSrcOptimal,
	                                          .dstImage = destination,
	                                          .dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
	                                          .regionCount = 1U,
	                                          .filter = vk::Filter::eLinear}
	                           .setRegions(blit_region);
	
	cmd_buffer.blitImage2(blit_info);
}
} // namespace caldera
