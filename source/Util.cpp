module;

#include <quill/LogMacros.h>

export module Caldera:Util;

import vulkan;
import QuillStatic;

using namespace std::literals;

constexpr auto check_if_success = [](vk::Result const& result,
                                     std::source_location const& src_loc = std::source_location::current()) -> void {
	if (result != vk::Result::eSuccess) {
		LOG_ERROR(logger, "Vulkan error at {}:{} - {}", src_loc.file_name(), src_loc.line(), vk::to_string(result));
		std::exit(EXIT_FAILURE);
	}
};
