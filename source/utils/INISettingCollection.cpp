#include "utils/INISettingCollection.h"

#include "utils/Logger.h"

namespace utils
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
	}

	bool INISettingCollection::ReadFromFile(std::string_view a_fileName)
	{
		std::filesystem::path iniPath = std::filesystem::current_path().append("Data\\SKSE\\Plugins").append(a_fileName);

		// Reference: decompiled source code in 1.5.97
		if (iniPath.string().c_str()) {
			if (iniPath.string().c_str() != subKey) {
				strcpy_s(subKey, iniPath.string().c_str());
			}
		} else {
			subKey[0] = '\0';
		}

		if (_this()->OpenHandle(false)) {
			_this()->Unk_09();
			_this()->CloseHandle();

			return true;
		}

		return false;
	}
}