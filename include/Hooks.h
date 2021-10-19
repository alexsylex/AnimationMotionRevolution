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
			SKSE::AllocTrampoline(32);

			constexpr REL::ID Activate(static_cast<std::uint64_t>(58602));
			constexpr REL::ID Deactivate(static_cast<std::uint64_t>(58604));

			unk_A0F480 = SKSE::GetTrampoline().write_call<5>(Activate.address() + 0x66E,  // E8 8D 32 00 00		call    hkbClipGenerator::unk_A0F480
															 unk_A0F480_Hook);

			unk_A0F610 = SKSE::GetTrampoline().write_call<5>(Deactivate.address() + 0x1A,	// E8 E1 2F 00 00       call    hkbClipGenerator::unk_A0F610
															 unk_A0F610_Hook);
		};

	private:

		static inline const RE::hkaAnimation* GetBoundAnimation(RE::hkbClipGenerator* a_this) 
		{ 
			return a_this->binding? (a_this->binding->animation? a_this->binding->animation.get() : nullptr) : nullptr;
		}

		static std::uint32_t unk_A0F480_Hook(RE::hkbClipGenerator* a_this);

		static inline REL::Relocation<decltype(&unk_A0F480_Hook)> unk_A0F480;

		static void unk_A0F610_Hook(RE::hkbClipGenerator* a_this);

		static inline REL::Relocation<decltype(&unk_A0F610_Hook)> unk_A0F610;
	};

	class MotionDataContainer
	{
		friend class Character;

		static RE::hkbCharacter* GethkbCharacter(RE::Character* a_character);

		// Translation
		static void unk_4DD5A0_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiPoint3& a_translation);

		static inline REL::Relocation<void(*)(std::uintptr_t*, float, RE::NiPoint3&)> ProcessTranslationData{ REL::ID(static_cast<std::uint64_t>(31812)) }; // Called from unk_4DD5A0

		// Rotation
		static void unk_4DD5F0_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiQuaternion& a_rotation);

		static inline REL::Relocation<void(*)(std::uintptr_t*, float, RE::NiQuaternion&)> ProcessRotationData{ REL::ID(static_cast<std::uint64_t>(31813)) }; // Called from unk_4DD5F0

		static inline REL::Relocation<void(*)(RE::NiQuaternion&, float, const RE::NiQuaternion&, const RE::NiQuaternion&)> InterpolateRotation{ REL::ID(static_cast<std::uint64_t>(69459)) }; // Called from ProcessRotationData
	};

	class Character
	{
	public:

		static void InstallHooks() 
		{
			SKSE::AllocTrampoline(64);

			constexpr REL::ID unk_4E6360(static_cast<std::uint64_t>(31949));
			
			SKSE::GetTrampoline().write_call<5>(unk_4E6360.address() + 0x28D,  // E8 AE 6F FF FF       call    unk_4DD5A0
												MotionDataContainer::unk_4DD5A0_Hook);

			SKSE::GetTrampoline().write_call<5>(unk_4E6360.address() + 0x2A1,	 // E8 9A 6F FF FF       call    unk_4DD5A0
												MotionDataContainer::unk_4DD5A0_Hook);

			SKSE::GetTrampoline().write_call<5>(unk_4E6360.address() + 0x355,	// E8 36 6F FF FF       call    unk_4DD5F0
												MotionDataContainer::unk_4DD5F0_Hook);
			
			SKSE::GetTrampoline().write_call<5>(unk_4E6360.address() + 0x368,	// E8 23 6F FF FF       call    unk_4DD5F0
												MotionDataContainer::unk_4DD5F0_Hook);
		}
	};
}
