#include <quill/LogMacros.h>

import Caldera;

auto main() -> int
{
	setup_quill();
	LOG_INFO(logger, "Starting engine");
	auto&& engine = caldera::Engine::getInstance(1920, 1080, "caldera");
	engine.run();
	LOG_INFO(logger, "ending");
}
