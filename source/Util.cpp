module;

#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/Logger.h>
#include <quill/LogMacros.h>
#include <quill/sinks/ConsoleSink.h>
#include <quill/sinks/FileSink.h>

export module Caldera:Util;

import vulkan;

using namespace std::literals;

export {
quill::Logger* logger;

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
constexpr auto USE_VALIDATION = true;
constexpr auto DEBUG_MODE = true;
#else
constexpr auto USE_VALIDATION = false;
constexpr auto DEBUG_MODE = false;
#endif

auto setup_quill(std::string_view const log_file = ""sv)
{
	if (logger != nullptr) {
		return;
	}

	static auto const log_format = quill::PatternFormatterOptions{
	    "[%(time)] [%(thread_id)] [%(log_level)] %(short_source_location:<20) %(message)"s, "%F %T.%Qms%z"s,
	    quill::Timezone::GmtTime};

	quill::Backend::start();

	// if no log file, log to console
	if (log_file.empty()) {
		auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_id_0"s);
		logger = quill::Frontend::create_or_get_logger("root"s, std::move(console_sink), log_format);
	} else {
		auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(
		    log_file.data(),
		    [] {
			    auto config = quill::FileSinkConfig{};
			    config.set_open_mode('w');
			    config.set_filename_append_option(quill::FilenameAppendOption::StartDateTime);
			    return config;
		    }(),
		    quill::FileEventNotifier{});
		logger = quill::Frontend::create_or_get_logger("root"s, std::move(file_sink), log_format);
	}

	if constexpr (DEBUG_MODE) {
		logger->set_log_level(quill::LogLevel::TraceL3);
	} else {
		logger->set_log_level(quill::LogLevel::Warning);
	}
}

constexpr auto vk_check = [](vk::Result const result, std::string_view const file, std::size_t const line) {
	if (result != vk::Result::eSuccess) {
		LOG_ERROR(logger, "Vulkan error at {}:{} - {}", file.data(), line, vk::to_string(result));
	}
};
}
