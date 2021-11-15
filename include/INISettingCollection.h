#pragma once

#include "RE/I/INISettingCollection.h"

#include "Setting.h"

namespace AMR
{
	class INISettingCollection
	{
	public:

		static AMR::INISettingCollection* GetSingleton()
		{
			static INISettingCollection singleton;

			return &singleton;
		}

		bool ReadFromFile(const char* a_fileName)
		{
			std::filesystem::path iniPath = std::filesystem::current_path().append("Data\\SKSE\\Plugins").append(a_fileName);

			return This()->ReadFromFile(iniPath.string().c_str());
		}

	private:

		INISettingCollection();

		// Virtual table auto-generation. Not real signatures. Not to be used directly.
		virtual ~INISettingCollection() = default;	// 00
		virtual void InsertSetting() {}				// 01
		virtual void RemoveSetting() {}				// 02
		virtual void WriteSetting() {}				// 03
		virtual void ReadSetting() {}				// 04
		virtual void OpenHandle() {}				// 05
		virtual void CloseHandle() {}				// 06
		virtual void Unk_07() {}					// 07
		virtual void Unk_08() {}					// 08
		virtual void Unk_09() {}					// 09

		RE::INISettingCollection* This() { return reinterpret_cast<RE::INISettingCollection*>(this); }

		// members
		char subKey[MAX_PATH];					  // 008
		std::uint32_t pad10C;					  // 10C
		void* handle;							  // 110
		RE::BSSimpleList<RE::Setting*> settings;  // 118

	};
	static_assert(sizeof(INISettingCollection) == sizeof(RE::INISettingCollection));
}
