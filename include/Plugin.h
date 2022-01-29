#pragma once

struct Plugin
{
	const char* name;
	std::uint16_t versionMajor = PLUGIN_VERSION_MAJOR;
	std::uint16_t versionMinor = PLUGIN_VERSION_MINOR;
	std::uint16_t versionPatch = PLUGIN_VERSION_PATCH;
	const char* fileName = PLUGIN_FILENAME;
};