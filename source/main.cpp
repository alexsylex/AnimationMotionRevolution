#include "Plugin.h"
#include "Hooks.h"

#include "utils/INISettingCollection.h"
#include "utils/Logger.h"

static constexpr Plugin plugin{ "Animation Motion Revolution" };

#if BUILD_SE
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = plugin.name;
	a_info->version = a_info->version = (static_cast<std::uint8_t>(plugin.versionMajor) << 24) |
										(static_cast<std::uint8_t>(plugin.versionMinor) << 16) |
										(static_cast<std::uint8_t>(plugin.versionPatch) << 8);

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible");
		return false;
	}

	REL::Version runtimeVersion = a_skse->RuntimeVersion();

	if (runtimeVersion < SKSE::RUNTIME_1_5_39 || runtimeVersion > SKSE::RUNTIME_1_5_97) {
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
}
();
#endif

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	using namespace utils;

	if (!logger::init(plugin.name)) {
		return false;
	}

	logger::info("{} {}.{}", plugin.name, plugin.versionMajor, plugin.versionMinor);

	SKSE::Init(a_skse);

	INISettingCollection* iniSettingCollection = INISettingCollection::GetSingleton();
	iniSettingCollection->AddSettings(
		MakeSetting("bEnableLog:Debug", false),
		MakeSetting("uLogLevel:Debug", static_cast<std::uint32_t>(logger::level::err)));

	std::string iniFileName = std::string(plugin.fileName) + ".ini";
	if (!iniSettingCollection->ReadFromFile(iniFileName)) {
		logger::warn("Could not load {}", iniFileName);
	}

	bool enableLog = iniSettingCollection->GetSetting<bool>("bEnableLog:Debug");
	auto loggerLevel = !enableLog ? logger::level::err :
									  static_cast<logger::level>(iniSettingCollection->GetSetting<std::uint32_t>("uLogLevel:Debug"));
	logger::set_level(loggerLevel, loggerLevel);

	AMR::InstallHooks();

	logger::set_level(logger::level::info, logger::level::info);
	logger::info("{} succesfully loaded", plugin.name);

	logger::set_level(loggerLevel, loggerLevel);

	return true;
}
