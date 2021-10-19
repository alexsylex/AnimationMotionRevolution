#include "INISettingCollection.h"

namespace AMR
{
	INISettingCollection::INISettingCollection()
	{
		REL::Relocation<std::uintptr_t> vTable(*reinterpret_cast<std::uintptr_t*>(this));

		// Use the Skyrim's INISettingCollection virtual functions
		vTable.write_vfunc(1, REL::ID(static_cast<std::uint64_t>(13054)).address());  // InsertSetting
		vTable.write_vfunc(2, REL::ID(static_cast<std::uint64_t>(13119)).address());  // RemoveSetting
		vTable.write_vfunc(3, REL::ID(static_cast<std::uint64_t>(74236)).address());  // WriteSetting
		vTable.write_vfunc(4, REL::ID(static_cast<std::uint64_t>(74237)).address());  // ReadSetting
		vTable.write_vfunc(5, REL::ID(static_cast<std::uint64_t>(74239)).address());  // OpenHandle
		vTable.write_vfunc(6, REL::ID(static_cast<std::uint64_t>(74240)).address());  // CloseHandle
		vTable.write_vfunc(7, REL::ID(static_cast<std::uint64_t>(10974)).address());  // Unk_07
		vTable.write_vfunc(8, REL::ID(static_cast<std::uint64_t>(13135)).address());  // Unk_08
		vTable.write_vfunc(9, REL::ID(static_cast<std::uint64_t>(13113)).address());  // Unk_09

		This()->InsertSetting(MakeSetting("bEnableLog:Debug", false));
		This()->InsertSetting(MakeSetting("sAmazingString:Debug", ""));
		This()->InsertSetting(MakeSetting("uDebugLevel:Debug", 0u));
	}
}