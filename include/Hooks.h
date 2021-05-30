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

		static std::uint32_t sub_140A0F480_Hook(RE::hkbClipGenerator* a_this);

		static inline REL::Relocation<decltype(&sub_140A0F480_Hook)> sub_140A0F480{ REL::ID(static_cast<std::uint64_t>(58628)) };

		static void sub_140A0F610_Hook(RE::hkbClipGenerator* a_this);

		static inline REL::Relocation<decltype(&sub_140A0F610_Hook)> sub_140A0F610{ REL::ID(static_cast<std::uint64_t>(58629)) };
	};

	class MotionDataContainer
	{
		friend class Character;

		static void sub_1404DD5A0_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiPoint3& a_translation);

		static inline REL::Relocation<decltype(&sub_1404DD5A0_Hook)> sub_1404DD5A0{ REL::ID(static_cast<std::uint64_t>(31804)) }; // Called from Character::sub_1404E6360

		static inline REL::Relocation<void(*)(std::uintptr_t*, float, RE::NiPoint3&)> ProcessTranslationData{ REL::ID(static_cast<std::uint64_t>(31812)) }; // Called from sub_1404DD5A0

		static void sub_1404DD5F0_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiQuaternion& a_rotation);

		static inline REL::Relocation<decltype(&sub_1404DD5F0_Hook)> sub_1404DD5F0{ REL::ID(static_cast<std::uint64_t>(31805)) }; // Called from Character::sub_1404E6360

		static inline REL::Relocation<void(*)(std::uintptr_t*, float, RE::NiQuaternion&)> ProcessRotationData{ REL::ID(static_cast<std::uint64_t>(31813)) }; // Called from sub_1404DD5F0

		static inline REL::Relocation<void(*)(RE::NiQuaternion&, float, const RE::NiQuaternion&, const RE::NiQuaternion&)> InterpolateRotation{ REL::ID(static_cast<std::uint64_t>(69459)) }; // Called from ProcessRotationData
	};

	class Character
	{
	public:

		static void InstallHooks()
		{
			constexpr REL::ID sub_1404E6360(static_cast<std::uint64_t>(31949));

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(sub_1404E6360.address() + 0x28D,	// E8 AE 6F FF FF       call    MotionDataContainer__c_ProcessTranslationData_1404DD5A0
												MotionDataContainer::sub_1404DD5A0_Hook);

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(sub_1404E6360.address() + 0x2A1,	// E8 9A 6F FF FF       call    MotionDataContainer__c_ProcessTranslationData_1404DD5A0
												MotionDataContainer::sub_1404DD5A0_Hook);

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(sub_1404E6360.address() + 0x355,	// E8 36 6F FF FF       call    MotionDataContainer__c_ProcessRotationData_1404DD5F0
												MotionDataContainer::sub_1404DD5F0_Hook);
			
			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(sub_1404E6360.address() + 0x368,	// E8 23 6F FF FF       call    MotionDataContainer__c_ProcessRotationData_1404DD5F0
												MotionDataContainer::sub_1404DD5F0_Hook);
		}
	};
}
