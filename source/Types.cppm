module;

#include <spdlog/spdlog.h>

export module Caldera.Types;

import std;
import vulkan_hpp;

export namespace caldera {
constexpr inline auto check = [](auto&& value) {
	if (value) {
		spdlog::log("Detected Vulkan error: {}", value);
	}
};
} // namespace caldera
