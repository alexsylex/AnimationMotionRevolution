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
			// hkbClipGenerator::Activate(hkbClipGenerator* this, hkbContext*)
			constexpr REL::ID Activate(static_cast<std::uint64_t>(58602));

			SKSE::AllocTrampoline(16);

			SKSE::GetTrampoline().write_call<5>(Activate.address() + 0x66E,	// E8 8D 32 00 00		call    hkbClipGenerator::unk_A0F480
												unk_A0F480_Hook);

			// hkbClipGenerator::Deactivate_140A0C610(hkbClipGenerator* this, hkbContext*)
			constexpr REL::ID Deactivate(static_cast<std::uint64_t>(58604));

			SKSE::AllocTrampoline(16);

			SKSE::GetTrampoline().write_call<5>(Deactivate.address() + 0x1A, // E8 E1 2F 00 00       call    hkbClipGenerator::unk_A0F610
												unk_A0F610_Hook);
		};

	private:

		static std::uint32_t unk_A0F480_Hook(RE::hkbClipGenerator* a_this);

		static inline REL::Relocation<decltype(&unk_A0F480_Hook)> unk_A0F480{ REL::ID(static_cast<std::uint64_t>(58628)) };

		static void unk_A0F610_Hook(RE::hkbClipGenerator* a_this);

		static inline REL::Relocation<decltype(&unk_A0F610_Hook)> unk_A0F610{ REL::ID(static_cast<std::uint64_t>(58629)) };
	};

	class MotionDataContainer
	{
		friend class Character;

		static void unk_4DD5A0_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiPoint3& a_translation);

		static inline REL::Relocation<decltype(&unk_4DD5A0_Hook)> unk_4DD5A0{ REL::ID(static_cast<std::uint64_t>(31804)) }; // Called from Character::unk_4E6360

		static inline REL::Relocation<void(*)(std::uintptr_t*, float, RE::NiPoint3&)> ProcessTranslationData{ REL::ID(static_cast<std::uint64_t>(31812)) }; // Called from unk_4DD5A0

		static void unk_4DD5F0_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiQuaternion& a_rotation);

		static inline REL::Relocation<decltype(&unk_4DD5F0_Hook)> unk_4DD5F0{ REL::ID(static_cast<std::uint64_t>(31805)) }; // Called from Character::unk_4E6360

		static inline REL::Relocation<void(*)(std::uintptr_t*, float, RE::NiQuaternion&)> ProcessRotationData{ REL::ID(static_cast<std::uint64_t>(31813)) }; // Called from unk_4DD5F0

		static inline REL::Relocation<void(*)(RE::NiQuaternion&, float, const RE::NiQuaternion&, const RE::NiQuaternion&)> InterpolateRotation{ REL::ID(static_cast<std::uint64_t>(69459)) }; // Called from ProcessRotationData
	};

	class Character
	{
	public:

		static void InstallHooks()
		{
			constexpr REL::ID unk_4E6360(static_cast<std::uint64_t>(31949));

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(unk_4E6360.address() + 0x28D,	// E8 AE 6F FF FF       call    unk_4DD5A0
												MotionDataContainer::unk_4DD5A0_Hook);

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(unk_4E6360.address() + 0x2A1,	// E8 9A 6F FF FF       call    unk_4DD5A0
												MotionDataContainer::unk_4DD5A0_Hook);

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(unk_4E6360.address() + 0x355,	// E8 36 6F FF FF       call    unk_4DD5F0
												MotionDataContainer::unk_4DD5F0_Hook);
			
			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(unk_4E6360.address() + 0x368,	// E8 23 6F FF FF       call    unk_4DD5F0
												MotionDataContainer::unk_4DD5F0_Hook);
		}
	};
}
