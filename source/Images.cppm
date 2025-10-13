module Caldera:Images;

import std;
import vulkan_hpp;

namespace caldera::util
{
auto transitionImage(vk::raii::CommandBuffer const& command, // NOLINT(*-use-internal-linkage)
                     vk::Image const& image,
                     vk::ImageLayout const current_layout,
                     vk::ImageLayout const new_layout)
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

auto copyImageToImage(vk::raii::CommandBuffer const& buffer,
                      vk::Image const& source,
                      vk::Image const& destination,
                      vk::Extent2D const& source_extent,
                      vk::Extent2D const& destination_extent) -> void
{
	constexpr auto sub_resource =
		vk::ImageSubresourceLayers{.aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1U};
	auto const blit_region = vk::ImageBlit2{
		.srcSubresource = sub_resource,
		.srcOffsets = {{
			{{},
	     {.x = static_cast<std::int32_t>(source_extent.width),  // NOLINT(*-narrowing-conversions)
	      .y = static_cast<std::int32_t>(source_extent.height), // NOLINT(*-narrowing-conversions)
	      .z = 1U}},
		}},
		.dstSubresource = sub_resource,
		.dstOffsets = {{{{},
	                   {.x = static_cast<std::int32_t>(destination_extent.width),  // NOLINT(*-narrowing-conversions)
	                    .y = static_cast<std::int32_t>(destination_extent.height), // NOLINT(*-narrowing-conversions)
	                    .z = 1U}}}},
	};

	auto const blit_info = vk::BlitImageInfo2{.srcImage = source,
	                                          .srcImageLayout = vk::ImageLayout::eTransferSrcOptimal,
	                                          .dstImage = destination,
	                                          .dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
	                                          .regionCount = 1U,
	                                          .filter = vk::Filter::eLinear}
	                         .setRegions(blit_region);

	buffer.blitImage2(blit_info);
}
} // namespace caldera::util
