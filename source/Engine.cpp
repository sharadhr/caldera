module;

#include <quill/LogMacros.h>

module Caldera;

import :Util;

import QuillStatic;
import vk_mem_alloc;
import vulkan;

namespace caldera
{
auto Engine::getInstance() -> Engine&
{
	LOG_INFO(global_logger, "static init of engine");
	
	static auto engine = Engine{};
	return engine;
}

auto Engine::run() -> void { LOG_INFO(global_logger, "running engine"); }
} // namespace caldera
