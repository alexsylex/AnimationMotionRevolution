#pragma once

#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>

#ifndef NDEBUG
#include <spdlog/sinks/msvc_sink.h>
#endif

#define DLLEXPORT __declspec(dllexport)

using namespace std::literals;

template <typename T>
constexpr auto full_type_name() -> std::string_view
{
	std::string_view funcsig = __FUNCSIG__;

	size_t startpos = funcsig.find('<') + 1;
	size_t endpos = funcsig.find('>');
	size_t len = endpos - startpos;

	return funcsig.substr(startpos, len);
}

template <typename T>
constexpr auto type_name() -> std::string_view
{
	constexpr std::string_view funcsig = __FUNCSIG__;

	constexpr size_t openbracketpos = funcsig.find('<');
	constexpr size_t spacepos = funcsig.find(' ', openbracketpos);
	constexpr size_t scopepos = funcsig.rfind(':');

	constexpr size_t startpos = (scopepos != funcsig.npos)? scopepos + 1 :
		(spacepos != funcsig.npos)? spacepos + 1 :
		openbracketpos;

	constexpr size_t endpos = funcsig.find('>');
	constexpr size_t len = endpos - startpos;

	return funcsig.substr(startpos, len);
}

namespace logger = SKSE::log;
