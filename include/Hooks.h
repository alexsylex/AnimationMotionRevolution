#pragma once

#include <xbyak/xbyak.h>

#include "RE/B/BShkbAnimationGraph.h"
#include "RE/H/hkbClipGenerator.h"
#include "RE/H/hkbContext.h"

#include "REL/Relocation.h"

#include "MotionDataContainer.h"

namespace AMR
{
	class hkbClipGenerator
	{
	public:

		static void InstallHooks()
		{
			// hkbClipGenerator::Activate_140A0BB80(hkbClipGenerator* this, hkbContext*)
			constexpr REL::ID Activate(static_cast<std::uint64_t>(58602));

			SKSE::AllocTrampoline(16);

			SKSE::GetTrampoline().write_call<5>(Activate.address() + 0x66E,	// E8 8D 32 00 00		call    hkbClipGenerator__sub_140A0F480
												sub_140A0F480_Hook);

			// hkbClipGenerator::Deactivate_140A0C610(hkbClipGenerator* this, hkbContext*)
			constexpr REL::ID Deactivate(static_cast<std::uint64_t>(58604));

			SKSE::AllocTrampoline(16);

			SKSE::GetTrampoline().write_call<5>(Deactivate.address() + 0x1A, // E8 E1 2F 00 00       call    hkbClipGenerator__sub_140A0F610
												sub_140A0F610_Hook);
		};

	private:

		static std::uint32_t __fastcall sub_140A0F480_Hook(RE::hkbClipGenerator* a_this);

		static inline REL::Relocation<decltype(&sub_140A0F480_Hook)> sub_140A0F480{ REL::ID(static_cast<std::uint64_t>(58628)) };

		static void __fastcall sub_140A0F610_Hook(RE::hkbClipGenerator* a_this);

		static inline REL::Relocation<decltype(&sub_140A0F610_Hook)> sub_140A0F610{ REL::ID(static_cast<std::uint64_t>(58629)) };
	};

	template<typename T>
	class MotionDataContainer;

	template<>
	class MotionDataContainer<RE::NiPoint3>
	{
		friend class Character;

		static void __fastcall sub_1404DD5A0_Hook(RE::MotionPositionContainer* a_this, float a_motionTime, RE::NiPoint3* a_pos);

		using func_t = decltype(&sub_1404DD5A0_Hook);

		static inline REL::Relocation<func_t> sub_1404DD5A0{ REL::ID(static_cast<std::uint64_t>(31804)) }; // Called from Character::sub_1404E6360

		static inline REL::Relocation<func_t> ApplyPositionData{ REL::ID(static_cast<std::uint64_t>(31812)) }; // Called from sub_1404DD5A0
	};

	using MotionPositionContainer = MotionDataContainer<RE::NiPoint3>;

	//template<>
	//class MotionDataContainer<RE::NiQuaternion>
	//{
	//	friend class Character;
	//
	//	static void __fastcall sub_1404DD5F0_Hook(RE::MotionDataContainer<RE::NiQuaternion>* a_this, float a_motionTime, RE::NiQuaternion* a_rot);
	//
	//	using func_t = decltype(&sub_1404DD5F0_Hook);
	//
	//	static inline REL::Relocation<func_t> sub_1404DD5F0{ REL::ID(static_cast<std::uint64_t>(31805)) }; // Called from Character::sub_1404E6360
	//
	//	static inline REL::Relocation<func_t> ApplyRotationData{ REL::ID(static_cast<std::uint64_t>(31813)) }; // Called from sub_1404DD5F0
	//};

	class Character
	{
	public:

		static void InstallHooks()
		{
			constexpr REL::ID sub_1404E6360(static_cast<std::uint64_t>(31949));

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(sub_1404E6360.address() + 0x28D,	// E8 AE 6F FF FF       call    MotionDataContainer__c_ApplyPositionData_1404DD5A0
												MotionPositionContainer::sub_1404DD5A0_Hook);

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(sub_1404E6360.address() + 0x2A1,	// E8 9A 6F FF FF       call    MotionDataContainer__c_ApplyPositionData_1404DD5A0
												MotionPositionContainer::sub_1404DD5A0_Hook);

			//SKSE::AllocTrampoline(16);
			//SKSE::GetTrampoline().write_call<5>(sub_1404E6360.address() + 0x355,	// E8 36 6F FF FF       call    MotionDataContainer__c_ApplyRotationData_1404DD5F0
			//									MotionDataContainer<RE::NiQuaternion>::sub_1404DD5F0_Hook);
			//
			//SKSE::AllocTrampoline(16);
			//SKSE::GetTrampoline().write_call<5>(sub_1404E6360.address() + 0x368,	// E8 23 6F FF FF       call    MotionDataContainer__c_ApplyRotationData_1404DD5F0
			//									MotionDataContainer<RE::NiQuaternion>::sub_1404DD5F0_Hook);
		}
	};
}
