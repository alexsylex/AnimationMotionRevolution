#pragma once

#include "AutoTrampoline.h"

#include "RE/B/BShkbAnimationGraph.h"
#include "RE/H/hkbClipGenerator.h"
#include "RE/H/hkbContext.h"

#include "MotionDataContainer.h"

namespace AMR
{
	class hkbClipGenerator
	{
	public:

		static inline REL::Relocation<std::uintptr_t> Activate{ REL::ID{ 58602 } };
		static inline REL::Relocation<std::uint32_t (*)(const RE::hkbClipGenerator*)> ComputeStartTime;
		static std::uint32_t ComputeStartTime_Hook(const RE::hkbClipGenerator* a_this, const RE::hkbContext* a_context);

		static inline REL::Relocation<std::uintptr_t> Deactivate{ REL::ID{ 58604 } };
		static inline REL::Relocation<void (*)(const RE::hkbClipGenerator*)> ResetIgnoreStartTime;
		static void ResetIgnoreStartTime_Hook(const RE::hkbClipGenerator* a_this, const RE::hkbContext* a_hkbContext);

	private:

		static const RE::hkaAnimation* GetBoundAnimation(const RE::hkbClipGenerator* a_this) 
		{ 
			return a_this->binding? (a_this->binding->animation? a_this->binding->animation.get() : nullptr) : nullptr;
		}
	};

	class MotionDataContainer
	{
	public:

		// Translation
		static inline REL::Relocation<void (*)(std::uintptr_t*, float, RE::NiPoint3&)> ProcessTranslationData{ REL::ID(static_cast<std::uint64_t>(31812)) };  // Called from unk_4DD5A0
		static void ProcessTranslationData_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiPoint3& a_translation, const RE::BSFixedString* a_clipName);

		// Rotation
		static inline REL::Relocation<void (*)(std::uintptr_t*, float, RE::NiQuaternion&)> ProcessRotationData{ REL::ID(static_cast<std::uint64_t>(31813)) };									// Called from unk_4DD5F0
		static inline REL::Relocation<void (*)(RE::NiQuaternion&, float, const RE::NiQuaternion&, const RE::NiQuaternion&)> InterpolateRotation{ REL::ID(static_cast<std::uint64_t>(69459)) };	// Called from ProcessRotationData
		static void ProcessRotationData_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiQuaternion& a_rotation, const RE::BSFixedString* a_clipName);

	private:

		static RE::hkbCharacter* GethkbCharacter(RE::Character* a_character)
		{
			RE::BSAnimationGraphManagerPtr animGraph;

			a_character->GetAnimationGraphManager(animGraph);

			if (animGraph && animGraph->graphs[animGraph->activeGraph])
			{
				return &animGraph->graphs[animGraph->activeGraph]->characterInstance;
			}

			return nullptr;
		};
	};

	class Character
	{
	public:

		static inline REL::Relocation<std::uintptr_t> ProcessMotionData{ REL::ID{ 31949 } };
	};

	static inline void InstallHooks()
	{
		// hkbClipGenerator::Activate
		{
			struct Hook : Xbyak::CodeGenerator
			{
				Hook()
				{
					Xbyak::Label hookLabel;

					mov(rdx, r15);	// r15 = hkbContext*
					jmp(ptr[rip + hookLabel]);

					L(hookLabel);
					dq(reinterpret_cast<std::uintptr_t>(&hkbClipGenerator::ComputeStartTime_Hook));
				}
			};

			hkbClipGenerator::ComputeStartTime = SKSE::AutoTrampoline<5>(hkbClipGenerator::Activate.address() + 0x66E, Hook{});
		}

		// hkbClipGenerator::Deactivate
		{
			struct Hook : Xbyak::CodeGenerator
			{
				Hook()
				{
					Xbyak::Label hookLabel;

					mov(rdx, r14);	// r14 = hkbContext*
					jmp(ptr[rip + hookLabel]);

					L(hookLabel);
					dq(reinterpret_cast<std::uintptr_t>(&hkbClipGenerator::ResetIgnoreStartTime_Hook));
				}
			};

			hkbClipGenerator::ResetIgnoreStartTime = SKSE::AutoTrampoline<5>(hkbClipGenerator::Deactivate.address() + 0x1A, Hook{ });
		}

		// Character::ProcessMotionData
		{
			struct Hook : Xbyak::CodeGenerator
			{
				Hook(void* a_func)
				{
					Xbyak::Label hookLabel;

					mov(r9, rbx);  // rbx = BSFixedString*
					jmp(ptr[rip + hookLabel]);

					L(hookLabel);
					dq(reinterpret_cast<std::uintptr_t>(a_func));
				}
			};

			// Translation
			SKSE::AutoTrampoline<5>(Character::ProcessMotionData.address() + 0x28D, Hook{ &MotionDataContainer::ProcessTranslationData_Hook });
			SKSE::AutoTrampoline<5>(Character::ProcessMotionData.address() + 0x2A1, Hook{ &MotionDataContainer::ProcessTranslationData_Hook });

			// Rotation
			SKSE::AutoTrampoline<5>(Character::ProcessMotionData.address() + 0x355, Hook{ &MotionDataContainer::ProcessRotationData_Hook });
			SKSE::AutoTrampoline<5>(Character::ProcessMotionData.address() + 0x368, Hook{ &MotionDataContainer::ProcessRotationData_Hook });
		}
	}
}
