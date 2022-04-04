#pragma once

#include "RE/S/Setting.h"

#include <windows.h>

namespace utils
{
	// Deriving from RE::Setting does not compile as the virtual functions are not defined (Unresolved external symbols).
	// The best way I could find is to replicate that class layout into mine.
	class Setting
	{
		template <typename T>
		friend RE::Setting* MakeSetting(const char* a_name, T a_data);

		using Type = RE::Setting::Type;

		template <typename T>
		Setting(const char* a_name, T a_data)
			requires(std::same_as<T, const char*> ||
					 std::same_as<T, bool> ||
					 std::same_as<T, int> ||
					 std::same_as<T, unsigned int> ||
					 std::same_as<T, float>)
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
						std::size_t dataLen = std::strlen(a_data) + sizeof('\0');
						if (dataLen < MAX_PATH) 
						{
							data.s = new char[dataLen];
							strcpy_s(data.s, dataLen, a_data);

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

		// For the virtual table auto-generation.
		virtual bool Unk_01(void) { return false; }	 // 01

		Type GetType() const
		{
			return reinterpret_cast<const RE::Setting*>(this)->GetType();
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
