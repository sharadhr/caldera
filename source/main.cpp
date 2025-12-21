#include <quill/LogMacros.h>

import Caldera;
import QuillStatic;

auto main() -> int
{
	setup_quill();
	LOG_INFO(global_logger, "Starting engine");
	auto&& engine = caldera::Engine::getInstance();
	engine.run();
	LOG_INFO(global_logger, "ending");
}
