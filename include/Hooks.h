#pragma once

#include <xbyak/xbyak.h>

#include "Offsets.h"

#include "RE/B/BShkbAnimationGraph.h"
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
			using namespace RE::Offset::hkbClipGenerator;

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(Activate.address() + 0x66E,	// E8 8D 32 00 00		call    hkbClipGenerator__sub_140A0F480
												sub_140A0F480_Hook);
		};

	private:

		static uint32_t __fastcall sub_140A0F480_Hook(RE::hkbClipGenerator* a_this);

		using func_t = decltype(&sub_140A0F480_Hook);
		static inline REL::Relocation<func_t> sub_140A0F480{ RE::Offset::hkbClipGenerator::sub_140A0F480 };	

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

	class BSAnimationGraphManager;

	class BShkbAnimationGraph
	{
		friend class BSAnimationGraphManager;

		static bool __fastcall sub_140AF0360_Hook(RE::BShkbAnimationGraph* a_this, float a_fVal, uint64_t a_u64Val);

		using func_t = decltype(&sub_140AF0360_Hook);
		static inline REL::Relocation<func_t> sub_140AF0360{ RE::Offset::BShkbAnimationGraph::sub_140AF0360 };
	};

	class BSAnimationGraphManager
	{
	public:

		static void InstallHook()
		{
			using namespace RE::Offset::BSAnimationGraphManager;

			SKSE::AllocTrampoline(16);
			SKSE::GetTrampoline().write_call<5>(sub_140AE2E80.address() + 0x9E,	// E8 3D D4 00 00		call    BShkbAnimationGraph__sub_140AF0360
												BShkbAnimationGraph::sub_140AF0360_Hook);
		};
	};

	constexpr float vector[3] = { 0.f, 100.f, 0.f };
	constexpr uint8_t zero = 0;

	class ApplyMotionDataHook
	{
	public:

		static void Install()
		{
			SKSE::AllocTrampoline(256);

			struct MyHook_Code : Xbyak::CodeGenerator
			{
				MyHook_Code()
				{
					Xbyak::Label retnLabel;

					push(rax);

					jnz("loadVec");
					mov(rax, (uintptr_t)&zero);
					lea(rdx, ptr[rax]);
					jmp("func");

					L("loadVec");
					//lea(eax, ptr[rbx-1]);
					//lea(rax, ptr[rax+rax*2]);
					//lea(rdx, ptr[r8+rax*4]);
					mov(rax, (uintptr_t)&vector[0]);
					lea(rdx, ptr[rax]);

					L("func");
					mov(eax, ptr[rdx]);
					subss(xmm10, xmm9);
					mov(ptr[rsi], eax);
					lea(rcx, ptr[rbx + rbx * 2]);
					mov(eax, ptr[rdx + 0x4]);
					movaps(xmm0, xmm9);
					mov(ptr[rsi + 0x4], eax);
					movaps(xmm1, xmm9);
					mov(eax, ptr[rdx + 0x8]);
					mov(ptr[rsi + 0x8], eax);

					//test
					//push(rax);
					////mov(rdx, (uintptr_t)&x);	 // test
					////mov(eax, ptr[rdx]); // original
					//mov(rax, (uintptr_t)&x);  // replacement
					//subss(xmm10, xmm9);
					//mov(ptr[rsi], rax);	 // mov(ptr[rsi], eax);
					//lea(rcx, ptr[rbx + rbx * 2]);
					////mov(ptr[rdx + 0x4], (uintptr_t)&y);	 // test
					////mov(eax, ptr[rdx+0x4]); // original
					//mov(rax, (uintptr_t)&y);  // replacement
					//movaps(xmm0, xmm9);
					//mov(ptr[rsi + 0x4], rax);  // mov(ptr[rsi+0x4], eax);
					//movaps(xmm1, xmm9);
					////mov(ptr[rdx+0x8], (uintptr_t)&z); // test
					////mov(eax, ptr[rdx+0x8]); // original
					//mov(rax, (uintptr_t)&z);   // replacement
					//mov(ptr[rsi + 0x8], rax);  // mov(ptr[rsi+0x8], eax);


					movaps(xmm3, xmm10);
					mulss(xmm3, dword[rsi]);
					movaps(xmm2, xmm10);
					mulss(xmm10, dword[rsi + 0x8]);
					mulss(xmm2, dword[rsi + 0x4]);
					movss(dword[rsi + 0x8], xmm10);
					movss(dword[rsi], xmm3);
					movss(dword[rsi + 0x4], xmm2);

					//mulss(xmm9, dword [r8 + rcx * 4 + 8]); // z
					//mulss(xmm0, dword [r8 + rcx * 4]); // x
					//mulss(xmm1, dword [r8 + rcx * 4 + 4]); // y

					//mulss(xmm9, dword [a_x]);
					//mulss(xmm0, dword [a_y]);
					//mulss(xmm1, dword [a_z]);

					mov(rax, (uintptr_t)&vector[2]);
					mulss(xmm9, dword[rax]);
					mov(rax, (uintptr_t)&vector[0]);
					mulss(xmm0, dword[rax]);
					mov(rax, (uintptr_t)&vector[1]);
					mulss(xmm1, dword[rax]);

					pop(rax);

					//mulss(xmm9, dword [(std::size_t)std::addressof(x)]);
					//mulss(xmm0, dword [(std::size_t)std::addressof(y)]);
					//mulss(xmm1, dword [(std::size_t)std::addressof(z)]);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					//dq(MyHook.address() + 0x14);
					//dq(MyHook.address() + 0x5B);
					dq(ApplyMotionData.address() + 0x71);
				}


			};

			//void* codeBuf = SKSE::GetTrampolineInterface()->AllocateFromLocalPool(4096);

			//MyHook_Code code(codeBuf);
			MyHook_Code code;
			code.ready();

			auto& trampoline = SKSE::GetTrampoline();

			trampoline.write_branch<6>(ApplyMotionData.address(), trampoline.allocate(code));

			/**********************************************************************************************
			 **********************************************************************************************/

			//SKSE::AllocTrampoline(1 << 4);

			//SKSE::Trampoline& trampoline = SKSE::GetTrampoline();

			//std::uint64_t hookOffset = 0x12;	// jmp     sub_1404DDA20
			//									// rcx = MotionDataContainer*, xmm0 = float motionTime, r8 = position

			//trampoline.write_call<5>(RE::Offset::j_ApplyMotionData.address() + hookOffset, Hook);
		};

	private:

		static inline REL::Relocation<std::uintptr_t> ApplyMotionData{ RE::Offset::ApplyMotionData, 0x164 };
	};
}