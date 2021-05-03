#pragma once

#include "PCH.h"

namespace SKSE::log
{
	bool Initialize(const std::string& log_name);
}

namespace GFx
{
	namespace logger
	{
		struct info;
		struct error;

		bool RegisterGlobalFunctions(RE::GFxMovieView* a_view, RE::GFxValue* a_root);
	}
}