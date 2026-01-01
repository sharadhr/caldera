module;

module Caldera:Descriptors;

import :Util;
import vulkan;

namespace caldera
{
struct DescriptorLayoutBuilder
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings_;

	auto addBinding(std::uint32_t binding, vk::DescriptorType type) -> DescriptorLayoutBuilder&
	{
		bindings_.emplace_back(binding, type, 1);
		return *this;
	}

	auto clear() -> void { bindings_.clear(); }

	auto build(vk::raii::Device const& device,
	           vk::ShaderStageFlags shader_stages,
	           void const* p_next = {},
	           vk::DescriptorSetLayoutCreateFlags const flags = {})
	{
		std::ranges::for_each(bindings_, [&](auto&& binding) { binding.stageFlags |= shader_stages; });

		return device
		    .createDescriptorSetLayout(
		        vk::DescriptorSetLayoutCreateInfo{.pNext = p_next,
		                                          .flags = flags,
		                                          .bindingCount = static_cast<std::uint32_t>(bindings_.size())}
		            .setBindings(bindings_))
		    .value;
	}
};

struct DescriptorAllocator
{
	struct PoolSizeRatio
	{
		vk::DescriptorType type_;
		float ratio_;
	};

	std::vector<vk::DescriptorPoolSize> poolSizes_;
	vk::raii::DescriptorPool pool_;

	DescriptorAllocator(vk::raii::Device const& logical_device,
	                    std::uint32_t max_sets,
	                    std::span<PoolSizeRatio const> const pool_ratios) :
	    poolSizes_{pool_ratios | std::views::transform([&max_sets](PoolSizeRatio const& pool_ratio) {
		               return vk::DescriptorPoolSize{.type = pool_ratio.type_,
		                                             .descriptorCount =
		                                                 static_cast<std::uint32_t>(pool_ratio.ratio_ * max_sets)};
	               })
	               | std::ranges::to<decltype(poolSizes_)>()},
	    pool_{logical_device
	              .createDescriptorPool(vk::DescriptorPoolCreateInfo{
	                  .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, .maxSets = max_sets}
	                                        .setPoolSizes(poolSizes_))
	              .value}
	{}

	auto clearDescriptors() const { check_if_success(pool_.reset()); }

	auto allocate(vk::raii::Device const& logical_device, vk::raii::DescriptorSetLayout const& layout) const
	    -> std::vector<vk::raii::DescriptorSet>
	{
		return logical_device
		    .allocateDescriptorSets(
		        vk::DescriptorSetAllocateInfo{.descriptorPool = pool_, .descriptorSetCount = 1U}.setSetLayouts(*layout))
		    .value;
	}
};
} // namespace caldera
