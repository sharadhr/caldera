#include <quill/LogMacros.h>

import Caldera;
import QuillStatic;

auto main() -> int
{
	setup_quill();
	LOG_INFO(logger, "Starting engine");
	auto&& engine = caldera::Engine::getInstance(1920, 1080, "caldera");
	engine.run();
	LOG_INFO(logger, "Stopping process");
}
