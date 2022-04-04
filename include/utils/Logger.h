#pragma once

#include "PCH.h"

namespace SKSE::log
{
	using level = spdlog::level::level_enum;

	inline void set_level(level a_log_level, level a_flush_level)
	{
		spdlog::default_logger()->set_level(a_log_level);
		spdlog::default_logger()->flush_on(a_flush_level);
	}

	inline bool init(const std::string& a_log_name)
	{
		if (!log_directory())
		{
			return false;
		}

		std::filesystem::path path = *log_directory() / (a_log_name + ".log");
		std::shared_ptr<spdlog::logger> log = spdlog::basic_logger_mt("global log", path.string(), true);

		spdlog::set_default_logger(std::move(log));

		set_level(level::info, level::info);

		spdlog::set_pattern("%D - %H:%M:%S.%f [%^%l%$] %v");

		return true;
	}

	inline void flush()
	{
		spdlog::default_logger()->flush();
	}
}

namespace logger = SKSE::log;