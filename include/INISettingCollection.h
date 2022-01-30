#pragma once

#include "RE/I/INISettingCollection.h"

#include "Setting.h"

namespace AMR
{
	// Deriving from RE::INISettingCollection does not compile as the virtual functions are not defined (Unresolved external symbols).
	// The best way I could find is to replicate that class layout into mine.
	class INISettingCollection
	{
#if BUILD_SE
		static constexpr inline REL::ID vTableId{ 230108 };
#else
		static constexpr inline REL::ID vTableId{ 187074 };
#endif
	public:

		static AMR::INISettingCollection* GetSingleton()
		{
			static INISettingCollection singleton;

			return &singleton;
		}

		bool ReadFromFile(std::string_view a_fileName)
		{
			std::filesystem::path iniPath = std::filesystem::current_path().append("Data\\SKSE\\Plugins").append(a_fileName);

			// Reference: decompiled source code in 1.5.97
			if (iniPath.string().c_str())
			{
				if (iniPath.string().c_str() != subKey) 
				{
					strcpy_s(subKey, iniPath.string().c_str());
				}
			} else 
			{
				subKey[0] = '\0';
			}

			if (OpenHandle()) 
			{
				Unk_09();
				CloseHandle();

				return true;
			}

			return false;
		}

		template <typename T = RE::Setting*>
		T GetSetting(const char* a_name) { return This()->GetSetting(a_name); }
		template <>
		bool GetSetting<bool>(const char* a_name) { return This()->GetSetting(a_name)->GetBool(); }
		template <>
		float GetSetting<float>(const char* a_name) { return This()->GetSetting(a_name)->GetFloat(); }
		template <>
		std::int32_t GetSetting<std::int32_t>(const char* a_name) { return This()->GetSetting(a_name)->GetSInt(); }
		template <>
		RE::Color GetSetting<RE::Color>(const char* a_name) { return This()->GetSetting(a_name)->GetColor(); }
		template <>
		const char* GetSetting<const char*>(const char* a_name) { return This()->GetSetting(a_name)->GetString(); }
		template <>
		std::uint32_t GetSetting<std::uint32_t>(const char* a_name) { return This()->GetSetting(a_name)->GetUInt(); }

	private:

		INISettingCollection();

		virtual ~INISettingCollection() = default;	// 00

		// Virtual table auto-generation.
		virtual void InsertSetting(RE::Setting*) { throw(""); }	 // 01
		virtual void RemoveSetting(RE::Setting*) { throw(""); }	 // 02
		virtual bool WriteSetting(RE::Setting*) { throw(""); }	 // 03
		virtual bool ReadSetting(RE::Setting*) { throw(""); }	 // 04
		virtual bool OpenHandle(bool = false) { throw(""); }	 // 05
		virtual bool CloseHandle() { throw(""); }				 // 06
		virtual void Unk_07() { throw(""); }					 // 07
		virtual void Unk_08() { throw(""); }					 // 08
		virtual void Unk_09() { throw(""); }					 // 09

		RE::INISettingCollection* This() { return reinterpret_cast<RE::INISettingCollection*>(this); }

		// members
		char subKey[MAX_PATH];					  // 008
		std::uint32_t pad10C;					  // 10C
		void* handle;							  // 110
		RE::BSSimpleList<RE::Setting*> settings;  // 118

	};
	static_assert(sizeof(INISettingCollection) == sizeof(RE::INISettingCollection));
}
