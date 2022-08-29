#pragma once

#include "RE/I/INISettingCollection.h"

#include "Setting.h"

namespace utils
{
	// Deriving from RE::INISettingCollection does not compile as the virtual functions are not defined (Unresolved external symbols).
	// The best way I could find is to replicate that class layout into mine.
	class INISettingCollection
	{
		static constexpr REL::RelocationID vTableId = RELOCATION_ID(230108, 187074);

	public:
		static INISettingCollection* GetSingleton()
		{
			static INISettingCollection singleton;

			return &singleton;
		}

		template <typename Last>
		void AddSettings(Last a_last)
		{
			_this()->InsertSetting(a_last);
		}

		template <typename First, typename... Rest>
		void AddSettings(First a_first, Rest... a_rest)
		{
			_this()->InsertSetting(a_first);
			AddSettings(a_rest...);
		}

		template <typename T = RE::Setting*>
		T GetSetting(const char* a_name) const
		{
			return const_cast<RE::INISettingCollection*>(_this())->GetSetting(a_name);
		}

		template <>
		bool GetSetting<bool>(const char* a_name) const
		{
			return GetSetting(a_name)->GetBool();
		}
		template <>
		float GetSetting<float>(const char* a_name) const
		{
			return GetSetting(a_name)->GetFloat();
		}
		template <>
		std::int32_t GetSetting<std::int32_t>(const char* a_name) const
		{
			return GetSetting(a_name)->GetSInt();
		}
		template <>
		RE::Color GetSetting<RE::Color>(const char* a_name) const
		{
			return GetSetting(a_name)->GetColor();
		}
		template <>
		const char* GetSetting<const char*>(const char* a_name) const
		{
			return GetSetting(a_name)->GetString();
		}
		template <>
		std::uint32_t GetSetting<std::uint32_t>(const char* a_name) const
		{
			return GetSetting(a_name)->GetUInt();
		}

		bool ReadFromFile(std::string_view a_fileName);

	private:
		INISettingCollection() noexcept;

		// Virtual table auto-generation. Needed to replace the original class.
		virtual ~INISettingCollection() = default;	// 00

		virtual void InsertSetting(RE::Setting*) { throw(""); }	 // 01
		virtual void RemoveSetting(RE::Setting*) { throw(""); }	 // 02
		virtual bool WriteSetting(RE::Setting*) { throw(""); }	 // 03
		virtual bool ReadSetting(RE::Setting*) { throw(""); }	 // 04
		virtual bool OpenHandle(bool) { throw(""); }			 // 05
		virtual bool CloseHandle() { throw(""); }				 // 06
		virtual void Unk_07() { throw(""); }					 // 07
		virtual void Unk_08() { throw(""); }					 // 08
		virtual void Unk_09() { throw(""); }					 // 09

		RE::INISettingCollection* _this() { return reinterpret_cast<RE::INISettingCollection*>(this); }
		const RE::INISettingCollection* _this() const { return reinterpret_cast<const RE::INISettingCollection*>(this); }

		// members
		char subKey[MAX_PATH];					  // 008
		std::uint32_t pad10C;					  // 10C
		void* handle;							  // 110
		RE::BSSimpleList<RE::Setting*> settings;  // 118
	};
	static_assert(sizeof(INISettingCollection) == sizeof(RE::INISettingCollection));
}
