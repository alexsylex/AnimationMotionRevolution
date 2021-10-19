#pragma once

#include "RE/S/Setting.h"

#include <windows.h>

namespace AMR
{
	class Setting
	{
		template <typename T>
		friend RE::Setting* MakeSetting(const char* a_name, T a_data);

		using Type = RE::Setting::Type;

		template <typename T>
		Setting(const char* a_name, T a_data)
		{
			std::size_t nameLen = std::strlen(a_name) + sizeof('\0');

			if (nameLen < MAX_PATH) 
			{
				name = new char[nameLen];
				strcpy_s(name, nameLen, a_name);

				if constexpr (std::is_same_v<T, const char*>) 
				{
					if (GetType() == Type::kString) 
					{
						if (SetStringValue(a_data)) 
						{
							return;
						}
					}
				} 
				else if constexpr (std::is_same_v<T, bool>) 
				{
					if (GetType() == Type::kBool) 
					{
						data.b = a_data;

						return;
					}
				} 
				else if constexpr (std::is_same_v<T, int>) 
				{
					if (GetType() == Type::kSignedInteger) 
					{
						data.i = a_data;

						return;
					}
				}
				else if constexpr (std::is_same_v<T, unsigned int>) 
				{
					if (GetType() == Type::kUnsignedInteger) 
					{
						data.u = a_data;

						return;
					}
				}
				else if constexpr (std::is_same_v<T, float>) 
				{
					if (GetType() == Type::kFloat) 
					{
						data.f = a_data;

						return;
					}
				}

				delete[] name;
				name = nullptr;
			}
		}

		virtual ~Setting()	// 00
		{
			if (name) 
			{
				if (GetType() == Type::kString)
				{
					delete[] data.s;
				}

				delete[] name;
			}
		}

		virtual bool Unk_01(void) { return false; }	 // 01

		Type GetType() const
		{
			return reinterpret_cast<const RE::Setting*>(this)->GetType();
		}

		bool SetStringValue(const char* a_str)
		{
			std::size_t dataLen = std::strlen(a_str) + sizeof('\0');
			if (dataLen < MAX_PATH)
			{
				data.s = new char[dataLen];
				strcpy_s(data.s, dataLen, a_str);

				return true;
			}

			return false;
		}

		// members
		RE::Setting::Data data{};  // 08
		char* name;				   // 10
	};
	static_assert(sizeof(Setting) == sizeof(RE::Setting));

	template <typename T>
	RE::Setting* MakeSetting(const char* a_name, T a_data)
	{
		return reinterpret_cast<RE::Setting*>(new Setting{ a_name, a_data });
	}
}
