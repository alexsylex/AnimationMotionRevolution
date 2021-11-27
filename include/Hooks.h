#pragma once

#include <xbyak/xbyak.h>

#include "RE/B/BShkbAnimationGraph.h"
#include "RE/H/hkbClipGenerator.h"
#include "RE/H/hkbContext.h"

#include "REL/Relocation.h"

#include "MotionDataContainer.h"

template <std::size_t N>
std::uintptr_t get_call(std::uintptr_t a_src)
{
	const auto disp = reinterpret_cast<std::int32_t*>(a_src + N - 4);
	const auto nextOp = a_src + N;
	return nextOp + *disp;
}

namespace AMR
{
	class hkbClipGenerator
	{
	public:

		static void InstallHooks()
		{
			InstallActivateHook();
			InstallDeactivateHook();
		};

	private:

		static void InstallActivateHook()
		{
			constexpr REL::ID Activate(static_cast<std::uint64_t>(58602)); 

			SKSE::AllocTrampoline(32);
			auto& trampoline = SKSE::GetTrampoline();

			struct HookCode : Xbyak::CodeGenerator
			{
				HookCode()
				{
					Xbyak::Label hookLabel;
					Xbyak::Label retnLabel;

					mov(rdx, r15);	// r15 = hkbContext*
					jmp(ptr[rip + hookLabel]);

					L(hookLabel);
					dq(reinterpret_cast<std::uintptr_t>(&unk_A0F480_Hook));
				}
			} unk_A0F480_code;

			unk_A0F480_code.ready();

			unk_A0F480 = trampoline.write_call<5>(Activate.address() + 0x66E,			// E8 8D 32 00 00		call    hkbClipGenerator::unk_A0F480
												  trampoline.allocate(unk_A0F480_code));
		}

		static void InstallDeactivateHook()
		{
			constexpr REL::ID Deactivate(static_cast<std::uint64_t>(58604));

			SKSE::AllocTrampoline(32);
			auto& trampoline = SKSE::GetTrampoline();

			struct HookCode : Xbyak::CodeGenerator
			{
				HookCode()
				{
					Xbyak::Label hookLabel;
					Xbyak::Label retnLabel;

					mov(rdx, r14);	// r14 = hkbContext*
					jmp(ptr[rip + hookLabel]);

					L(hookLabel);
					dq(reinterpret_cast<std::uintptr_t>(&unk_A0F610_Hook));
				}
			} unk_A0F610_code;

			unk_A0F610_code.ready();

			unk_A0F610 = trampoline.write_call<5>(Deactivate.address() + 0x1A,			// E8 E1 2F 00 00       call    hkbClipGenerator::unk_A0F610
												  trampoline.allocate(unk_A0F610_code));
		}

		static const RE::hkaAnimation* GetBoundAnimation(const RE::hkbClipGenerator* a_this) 
		{ 
			return a_this->binding? (a_this->binding->animation? a_this->binding->animation.get() : nullptr) : nullptr;
		}

		// Activate
		static std::uint32_t unk_A0F480_Hook(const RE::hkbClipGenerator* a_this, const RE::hkbContext* a_context);

		static inline REL::Relocation<std::uint32_t (*)(const RE::hkbClipGenerator*)> unk_A0F480;

		// Deactivate
		static void unk_A0F610_Hook(const RE::hkbClipGenerator* a_this, const RE::hkbContext* a_hkbContext);

		static inline REL::Relocation<void(*)(const RE::hkbClipGenerator*)> unk_A0F610;
	};

	class MotionDataContainer
	{
		friend class Character;

		static RE::hkbCharacter* GethkbCharacter(RE::Character* a_character);

		// Translation
		static void unk_4DD5A0_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiPoint3& a_translation, const RE::BSFixedString* a_clipName);

		static inline REL::Relocation<void(*)(std::uintptr_t*, float, RE::NiPoint3&)> ProcessTranslationData{ REL::ID(static_cast<std::uint64_t>(31812)) }; // Called from unk_4DD5A0

		// Rotation
		static void unk_4DD5F0_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiQuaternion& a_rotation, const RE::BSFixedString* a_clipName);

		static inline REL::Relocation<void(*)(std::uintptr_t*, float, RE::NiQuaternion&)> ProcessRotationData{ REL::ID(static_cast<std::uint64_t>(31813)) }; // Called from unk_4DD5F0

		static inline REL::Relocation<void(*)(RE::NiQuaternion&, float, const RE::NiQuaternion&, const RE::NiQuaternion&)> InterpolateRotation{ REL::ID(static_cast<std::uint64_t>(69459)) }; // Called from ProcessRotationData
	};

	class Character
	{
	public:

		static void InstallHooks()
		{			
			constexpr REL::ID unk_4E6360(static_cast<std::uint64_t>(31949));

			SKSE::AllocTrampoline(128);
			auto& trampoline = SKSE::GetTrampoline();

			struct HookCode : Xbyak::CodeGenerator
			{
				HookCode(std::uintptr_t a_hookFunc)
				{
					Xbyak::Label hookLabel;
					Xbyak::Label retnLabel;
					//sub(rsp, 8);
					//mov(ptr[rsp], r13); // r13 = Character*
					mov(r9, rbx);		// rbx = BSFixedString*
					//push(rbp);
					jmp(ptr[rip + hookLabel]);

					L(hookLabel);
					dq(a_hookFunc);

					//add(rsp, 8);
				}
			};

			HookCode unk_4DD5A0_code{ reinterpret_cast<std::uintptr_t>(&MotionDataContainer::unk_4DD5A0_Hook) };
			unk_4DD5A0_code.ready();

			HookCode unk_4DD5F0_code{ reinterpret_cast<std::uintptr_t>(&MotionDataContainer::unk_4DD5F0_Hook) };
			unk_4DD5F0_code.ready();

			trampoline.write_call<5>(unk_4E6360.address() + 0x28D,	// E8 AE 6F FF FF       call    unk_4DD5A0
									 trampoline.allocate(unk_4DD5A0_code));

			trampoline.write_call<5>(unk_4E6360.address() + 0x2A1,	// E8 9A 6F FF FF       call    unk_4DD5A0
									 trampoline.allocate(unk_4DD5A0_code));

			trampoline.write_call<5>(unk_4E6360.address() + 0x355,	// E8 36 6F FF FF       call    unk_4DD5F0
									 trampoline.allocate(unk_4DD5F0_code));
			
			trampoline.write_call<5>(unk_4E6360.address() + 0x368,	// E8 23 6F FF FF       call    unk_4DD5F0
									 trampoline.allocate(unk_4DD5F0_code));
		}
	};
}
