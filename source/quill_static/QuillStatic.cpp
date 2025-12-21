module;

#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/Logger.h>
#include <quill/sinks/ConsoleSink.h>
#include <quill/sinks/FileSink.h>

export module QuillStatic;

import std;

export quill::Logger* global_logger;

export auto setup_quill(std::string_view const log_file = "")
{
	if (global_logger != nullptr) {
		return;
	}

	// TODO: customise this; the default format is ugly
	static auto const log_format =
	    quill::PatternFormatterOptions{"[%(time)] [%(thread_id)] [%(log_level)] %(short_source_location:<20) %(message)",
	                                   "%F %T.%Qms%z", quill::Timezone::GmtTime};

	quill::Backend::start();

	// if no log file, log to console
	if (log_file.empty()) {
		auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_id_0");
		global_logger = quill::Frontend::create_or_get_logger("root", std::move(console_sink), log_format);
	} else {
		constexpr auto make_config = [] {
			auto config = quill::FileSinkConfig{};
			config.set_open_mode('w');
			config.set_filename_append_option(quill::FilenameAppendOption::StartDateTime);
			return config;
		};
		auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(log_file.data(), make_config(),
		                                                                      quill::FileEventNotifier{});
		global_logger = quill::Frontend::create_or_get_logger("root", std::move(file_sink), log_format);
	}
}
