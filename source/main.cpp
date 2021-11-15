#include "Hooks.h"
#include "INISettingCollection.h"
#include "Logger.h"

struct PluginInfoEx : SKSE::PluginInfo
{
	std::uint32_t minorVersion = 0;
	std::uint32_t patchVersion = 0;
	const char* iniFileName = "AnimationMotionRevolution.ini";
};

static constexpr PluginInfoEx pluginInfo{ SKSE::PluginInfo::kVersion, "Animation Motion Revolution", 1, 4 };

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	*a_info = pluginInfo;

	if (!logger::init(pluginInfo.name))
	{
		return false;
	}

	logger::info("{} {}.{}", pluginInfo.name, pluginInfo.version, pluginInfo.minorVersion);

	if (a_skse->IsEditor())
	{
		logger::critical("Loaded in editor, marking as incompatible");

		return false;
	}

	REL::Version runtimeVersion = a_skse->RuntimeVersion();

	if (runtimeVersion < SKSE::RUNTIME_1_5_39)
	{
		logger::critical("Unsupported runtime version {}", runtimeVersion.string());

		return false;
	}

	if (!AMR::INISettingCollection::GetSingleton()->ReadFromFile(pluginInfo.iniFileName)) 
	{
		logger::warn("Could not load {}", pluginInfo.iniFileName);
	}

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	logger::info("{} succesfully loaded", pluginInfo.name);

	SKSE::Init(a_skse);

	// Hide messages about memory allocated by trampolines
	logger::set_level(logger::level::warn, logger::level::warn);

	AMR::hkbClipGenerator::InstallHooks();
	AMR::Character::InstallHooks();

	logger::set_level(logger::level::debug, logger::level::debug);

	return true;
}
