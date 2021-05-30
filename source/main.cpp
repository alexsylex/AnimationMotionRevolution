#include "Hooks.h"
#include "Logger.h"

static constexpr SKSE::PluginInfo plugin{ SKSE::PluginInfo::kVersion, "Animation Motion Revolution", 1 };

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	*a_info = plugin;

	if(!logger::Initialize(a_info->name))
	{
		return false;
	}

	logger::info("{} v{}", a_info->name, a_info->version);

	if(a_skse->IsEditor())
	{
		logger::critical("Loaded in editor, marking as incompatible");

		return false;
	}

	REL::Version runtimeVersion = a_skse->RuntimeVersion();

	if(runtimeVersion < SKSE::RUNTIME_1_5_39)
	{
		logger::critical("Unsupported runtime version {}", runtimeVersion.string());

		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	logger::info("{} loaded", plugin.name);
	logger::flush();

	SKSE::Init(a_skse);

	AMR::hkbClipGenerator::InstallHooks();
	AMR::Character::InstallHooks();

	return true;
}
