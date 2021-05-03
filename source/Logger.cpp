#include "Logger.h"

using namespace RE;

namespace SKSE::log
{
	bool Initialize(const std::string& log_name)
	{
		if(!log_directory())
		{
			return false;
		}

		std::filesystem::path path = *log_directory() / (log_name + ".log");
		std::shared_ptr<spdlog::logger> log = spdlog::basic_logger_mt("global log", path.string(), true);

		log->set_level(spdlog::level::info);
		log->flush_on(spdlog::level::warn);

		spdlog::set_default_logger(std::move(log));

		spdlog::set_pattern("%D - %H:%M:%S.%f [%^%l%$] %v");

		return true;
	}
}