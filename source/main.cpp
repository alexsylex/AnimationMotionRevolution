#include "Hooks.h"
#include "INISettingCollection.h"
#include "Logger.h"
#include "Plugin.h"

static constexpr Plugin plugin{ "Animation Motion Revolution" };

#if BUILD_SE
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = plugin.name;
	a_info->version = REL::Version{ plugin.versionMajor, plugin.versionMinor, plugin.versionPatch }.pack();

	if (a_skse->IsEditor()) 
	{
		logger::critical("Loaded in editor, marking as incompatible");
		return false;
	}

	REL::Version runtimeVersion = a_skse->RuntimeVersion();

	if (runtimeVersion < SKSE::RUNTIME_1_5_39 || runtimeVersion > SKSE::RUNTIME_1_5_97)
	{
		logger::critical("Unsupported runtime version {}", runtimeVersion.string());
		return false;
	}

	return true;
}
#else
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() constexpr -> SKSE::PluginVersionData 
{
	SKSE::PluginVersionData v;

	v.PluginVersion(REL::Version{ plugin.versionMajor, plugin.versionMinor, plugin.versionPatch });
	v.PluginName(plugin.name);

	v.UsesAddressLibrary(true);

	return v;
}();
#endif

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	if (!logger::init(plugin.fileName))
	{
		return false;
	}

	logger::info("{} {}.{}", plugin.name, plugin.versionMajor, plugin.versionMinor);

	SKSE::Init(a_skse);

	AMR::INISettingCollection* iniSettingCollection = AMR::INISettingCollection::GetSingleton();

	std::string iniFileName = std::string(plugin.fileName) + ".ini";
	if (!iniSettingCollection->ReadFromFile(iniFileName))
	{
		logger::warn("Could not load {}", iniFileName);
	}

	bool enableLog = iniSettingCollection->GetSetting<bool>("bEnableLog:Debug");
	if (enableLog)
	{
		auto loggerLevel = static_cast<logger::level>(iniSettingCollection->GetSetting<std::uint32_t>("uLogLevel:Debug"));
		logger::set_level(loggerLevel, loggerLevel);
	}
	else
	{
		logger::set_level(logger::level::err, logger::level::err);
	}

	AMR::InstallHooks();

	logger::info("{} succesfully loaded", plugin.name);

	return true;
}
