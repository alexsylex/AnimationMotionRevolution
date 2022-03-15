#include "INISettingCollection.h"

#include "Logger.h"

namespace alexsylex
{
	INISettingCollection::INISettingCollection() noexcept
	{
		REL::Relocation<std::uintptr_t> __vTable(*reinterpret_cast<std::uintptr_t*>(this));

		// Use the Skyrim's INISettingCollection virtual functions
		std::uintptr_t* skyrimsVTable = REL::Relocation<std::uintptr_t*>{ vTableId }.get();

		// Replace all except the destructor (index 0)
		for (int i = 1; i < 10; i++) {
			__vTable.write_vfunc(i, skyrimsVTable[i]);
		}

		_this()->InsertSetting(MakeSetting("bEnableLog:Debug", false));
		_this()->InsertSetting(MakeSetting("uLogLevel:Debug", static_cast<std::uint32_t>(logger::level::err)));
	}
}