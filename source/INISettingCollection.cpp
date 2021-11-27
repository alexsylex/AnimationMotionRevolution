#include "INISettingCollection.h"

#include "Logger.h"

namespace AMR
{
	INISettingCollection::INISettingCollection()
	{
		// Use the Skyrim's INISettingCollection virtual functions
		REL::Relocation<std::uintptr_t> vTable(*reinterpret_cast<std::uintptr_t*>(this));
		std::uintptr_t* originalVTable = REL::Relocation<std::uintptr_t*>{ REL::ID(230108) }.get();

		// Replace all except the destructor (index 0)
		for (int i = 1; i < 10; i++) 
		{
			vTable.write_vfunc(i, originalVTable[i]);
		}

		This()->InsertSetting(MakeSetting("bEnableLog:Debug", false));
		This()->InsertSetting(MakeSetting("uLogLevel:Debug", static_cast<std::uint32_t>(logger::level::err)));
	}
}