#include "Hooks.h"
#include "INISettingCollection.h"
#include "Logger.h"

struct PluginInfoEx : SKSE::PluginInfo
{
	std::uint32_t minorVersion;
	std::uint32_t patchVersion;
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

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	logger::info("{} succesfully loaded", pluginInfo.name);

	SKSE::Init(a_skse);

	AMR::INISettingCollection* iniSettingCollection = AMR::INISettingCollection::GetSingleton();

	if (!iniSettingCollection->ReadFromFile(pluginInfo.iniFileName)) 
	{
		logger::warn("Could not load {}", pluginInfo.iniFileName);
	} 

	bool enableLog = iniSettingCollection->GetSetting("bEnableLog:Debug")->GetBool();
	if (enableLog)
	{
		auto loggerLevel = static_cast<logger::level>(iniSettingCollection->GetSetting("uLogLevel:Debug")->GetUInt());
		logger::set_level(loggerLevel, loggerLevel);
	}
	else 
	{
		logger::set_level(logger::level::err, logger::level::err);
	}

	AMR::hkbClipGenerator::InstallHooks();
	AMR::Character::InstallHooks();

	return true;
}
