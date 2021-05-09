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

	class BShkbAnimationGraph
	{
		friend class BSAnimationGraphManager;

		static bool __fastcall sub_140AF0360_Hook(RE::BShkbAnimationGraph* a_this, float a_fVal, uint64_t a_u64Val);

		using func_t = decltype(&sub_140AF0360_Hook);
		static inline REL::Relocation<func_t> sub_140AF0360{ REL::ID(static_cast<std::uint64_t>(62649)) }; // Called from BSAnimationGraphManager::sub_140AE2E80
	};

	class BSAnimationGraphManager
	{
	public:

		static void InstallHook()
		{
			constexpr REL::ID sub_140AE2E80(static_cast<std::uint64_t>(62431));

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(sub_140AE2E80.address() + 0x9E,	// E8 3D D4 00 00		call    BShkbAnimationGraph__sub_140AF0360
												BShkbAnimationGraph::sub_140AF0360_Hook);
		};
	};

	class BSMotionDataContainer
	{
	public:

		static void InstallHook()
		{
			// uint32_t __fastcall sub_1404DD5A0(uint64_t a_motionDataContainer, float a_fMotionTime, NiPoint3 *a_pos)
			constexpr REL::ID j_ApplyMotionData(static_cast<std::uint64_t>(31804));

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_branch<5>(j_ApplyMotionData.address() + 0x12,	// E9 69 04 00 00		jmp     BSMotionDataContainer__ApplyMotionData_1404DDA20
												  ApplyMotionData_Hook);
		}

	private:

		static uint32_t __fastcall ApplyMotionData_Hook(RE::BSMotionDataContainer* a_this, float a_motionTime, RE::NiPoint3* a_pos);

		// uint32_t __fastcall sub_1404DDA20(BSMotionDataContainer* a_motionDataContainer, float a_fMotionTime, NiPoint3 * a_pos)
		using func_t = decltype(&ApplyMotionData_Hook);
		static inline REL::Relocation<func_t> ApplyMotionData{ REL::ID(static_cast<std::uint64_t>(31812)) }; // Called from j_ApplyMotionData
	};
}
