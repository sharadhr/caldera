module;

#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/Logger.h>
#include <quill/LogMacros.h>
#include <quill/sinks/ConsoleSink.h>

module Caldera:Logging;

import std;

namespace caldera::log
{
quill::Logger* logger{};

void setupQuill()
{
	quill::Backend::start();

	if (logger != nullptr) {
		return;
	}
	auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_id_0");

	logger = quill::Frontend::create_or_get_logger("root", std::move(console_sink));
	logger->set_log_level(quill::LogLevel::TraceL3);
}
} // namespace caldera::log
