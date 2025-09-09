module;

#include <spdlog/spdlog.h>

export module Caldera.Types;

import std;
import vulkan_hpp;

export namespace caldera
{
constexpr inline auto checkResult = [](vk::Result const& result) {
	if (result != vk::Result::eSuccess) {
		spdlog::error("Vulkan error: {}", vk::to_string(result));
	}
	return result == vk::Result::eSuccess;
};
} // namespace caldera
