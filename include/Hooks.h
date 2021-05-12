#pragma once

#include <xbyak/xbyak.h>

#include "RE/B/BShkbAnimationGraph.h"
#include "RE/B/BSMotionDataContainer.h"
#include "RE/H/hkbClipGenerator.h"
#include "RE/H/hkbContext.h"

#include "REL/Relocation.h"

namespace DMR
{
	class hkbClipGenerator
	{
	public:

		static void InstallHook()
		{
			// hkbClipGenerator::sub_140A0BB80(hkbClipGenerator* this, hkbContext*)
			constexpr REL::ID Activate(static_cast<std::uint64_t>(58602));

			SKSE::AllocTrampoline(16);

			SKSE::GetTrampoline().write_call<5>(Activate.address() + 0x66E,	// E8 8D 32 00 00		call    hkbClipGenerator__sub_140A0F480
												sub_140A0F480_Hook);
		};

	private:

		static uint32_t __fastcall sub_140A0F480_Hook(RE::hkbClipGenerator* a_this);

		using func_t = decltype(&sub_140A0F480_Hook);
		static inline REL::Relocation<func_t> sub_140A0F480{ REL::ID(static_cast<uint64_t>(58628)) };

		static inline RE::hkbContext* GethkbContext()
		{
			struct GethkbContext : Xbyak::CodeGenerator
			{
				GethkbContext()
				{
					mov(rax, r15); // r15 = hkbContext*
					ret();
				}
			} gethkbContext;

			return gethkbContext.getCode<RE::hkbContext* (*)()>()();
		}
	};

	class BSMotionDataContainer
	{
		friend class Character;

		static void __fastcall sub_1404DD5A0_Hook(RE::BSMotionDataContainer* a_this, float a_motionTime, RE::NiPoint3* a_pos);

		using func_t = decltype(&sub_1404DD5A0_Hook);
		static inline REL::Relocation<func_t> sub_1404DD5A0{ REL::ID(static_cast<std::uint64_t>(31804)) }; // Called from Character::sub_1404E6360

		static inline REL::Relocation<func_t> ApplyMotionData{ REL::ID(static_cast<std::uint64_t>(31812)) }; // Called from sub_1404DD5A0

		static inline RE::hkbCharacter* GethkbCharacter()
		{
			struct GetCharacter : Xbyak::CodeGenerator
			{
				GetCharacter()
				{
					mov(rax, r13); // r13 = Character*
					ret();
				}
			} getCharacter;

			auto character = getCharacter.getCode<RE::Character* (*)()>()();

			if(character)
			{
				RE::BSAnimationGraphManagerPtr animGraph;

				character->GetAnimationGraphManager(animGraph);

				if(animGraph)
				{
					return &animGraph->graphs[animGraph->activeGraph]->characterInstance;
				}
			}

			return nullptr;
		}
	};

	class Character
	{
	public:

		static void InstallHooks()
		{
			constexpr REL::ID sub_1404E6360(static_cast<std::uint64_t>(31949));

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(sub_1404E6360.address() + 0x28D,	// E8 AE 6F FF FF       call    BSMotionDataContainer__j_ApplyMotionData_1404DD5A0
												BSMotionDataContainer::sub_1404DD5A0_Hook);

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(sub_1404E6360.address() + 0x2A1,	// E8 9A 6F FF FF       call    BSMotionDataContainer__j_ApplyMotionData_1404DD5A0
												BSMotionDataContainer::sub_1404DD5A0_Hook);
		}
	};
}
