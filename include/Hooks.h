#pragma once

#include "utils/Trampoline.h"

#include "RE/B/bhkCharacterMoveInfo.h"
#include "RE/B/BShkbAnimationGraph.h"
#include "RE/H/hkbClipGenerator.h"
#include "RE/H/hkbContext.h"

#include "RE/M/MotionDataContainer.h"

namespace hooks
{
	class hkbClipGenerator
	{
		static constexpr REL::RelocationID ActivateId{ 58602, 59252 };
		static constexpr REL::RelocationID DeactivateId{ 58604, 59254 };

	public:

		static inline REL::Relocation<std::uintptr_t> Activate{ ActivateId }; 
		static inline REL::Relocation<std::uint32_t (*)(const RE::hkbClipGenerator*)> ComputeStartTime;
		static std::uint32_t ComputeStartTime_Hook(const RE::hkbClipGenerator* a_this, const RE::hkbContext* a_context);

		static inline REL::Relocation<std::uintptr_t> Deactivate{ DeactivateId };
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
		static constexpr REL::RelocationID ProcessTranslationDataId{ 31812, 32582 };
		static constexpr REL::RelocationID ProcessRotationDataId{ 31813, 32583 };
		static constexpr REL::RelocationID InterpolateRotationId{ 69459, 70836 };

	public:

		// Translation
		static inline REL::Relocation<void (*)(std::uintptr_t*, float, RE::NiPoint3&)> ProcessTranslationData{ ProcessTranslationDataId };
		static void ProcessTranslationData_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiPoint3& a_translation,
												const RE::BSFixedString* a_clipName, RE::Character* a_character);

		// Rotation
		static inline REL::Relocation<void (*)(std::uintptr_t*, float, RE::NiQuaternion&)> ProcessRotationData{ ProcessRotationDataId };
		static inline REL::Relocation<void (*)(RE::NiQuaternion&, float, const RE::NiQuaternion&, const RE::NiQuaternion&)> InterpolateRotation{ InterpolateRotationId };
		static void ProcessRotationData_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiQuaternion& a_rotation, 
											 const RE::BSFixedString* a_clipName, RE::Character* a_character);

	private:

		static RE::hkbCharacter* GethkbCharacter(RE::Character* a_character)
		{
			RE::BSAnimationGraphManagerPtr animGraph;

			a_character->GetAnimationGraphManager(animGraph);

			if (animGraph)
			{
				RE::BShkbAnimationGraphPtr activeGraph = animGraph->graphs[animGraph->GetRuntimeData().activeGraph];

				if (activeGraph)
				{
					return &activeGraph->characterInstance;
				}
			}

			return nullptr;
		};
	};

	class Character
	{	
		static constexpr REL::RelocationID ProcessMotionDataId{ 31949, 32703 };

	public:

		static inline REL::Relocation<std::uintptr_t> ProcessMotionData{ ProcessMotionDataId };
	};

	class bhkCharacterStateOnGround
	{
	public:

		static inline REL::Relocation<std::uintptr_t> vTable{ RE::VTABLE_bhkCharacterStateOnGround[0] };

		static inline REL::Relocation<void (*)(RE::bhkCharacterStateOnGround*, RE::bhkCharacterController*)> SimulateStatePhysics;
	};

	class bhkCharacterController
	{
		static constexpr REL::RelocationID UpdateStateAndMovementId{ 76436, 32703 };

	public:

		static inline REL::Relocation<RE::hkpCharacterStateType (*)(RE::bhkCharacterController*, RE::bhkCharacterMoveInfo*)> 
			UpdateStateAndMovement{ UpdateStateAndMovementId };
	};

	class hkpCharacterContext
	{
	public:

		static RE::hkpCharacterStateType GetCharacterState_Hook(RE::hkpCharacterContext* a_this);
	};

	void SimulateStatePhysics(RE::bhkCharacterStateOnGround* a_this, RE::bhkCharacterController* a_characterController);

	static inline void Install()
	{
		bhkCharacterStateOnGround::SimulateStatePhysics = bhkCharacterStateOnGround::vTable.write_vfunc(8, SimulateStatePhysics);

		// bhkCharacterController::UpdateStateAndMovement
		{
			static std::uintptr_t hookedAddress = bhkCharacterController::UpdateStateAndMovement.address() + 0x7D1;

			utils::AllocExactSizeTrampoline<5>();
			SKSE::GetTrampoline().write_call<5>(hookedAddress, &hkpCharacterContext::GetCharacterState_Hook);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// My guess is that Bethesda used compiled Havok libraries, that is why neither registers 
		// nor offsets changed between SE-AE for the hkbClipGenerator hooks

		// hkbClipGenerator::Activate
		{
			static std::uintptr_t hookedAddress = hkbClipGenerator::Activate.address() + 0x66E;

			struct Hook : Xbyak::CodeGenerator
			{
				Hook()
				{
					Xbyak::Label hookLabel;
					Xbyak::Label retnLabel;

					mov(rdx, r15);	// r15 = hkbContext*
					call(ptr[rip + hookLabel]);

					jmp(ptr[rip + retnLabel]);

					L(hookLabel), dq(reinterpret_cast<std::uintptr_t>(&hkbClipGenerator::ComputeStartTime_Hook));
					L(retnLabel), dq(hookedAddress + 5);
				}
			};

			hkbClipGenerator::ComputeStartTime = utils::WriteBranchTrampoline<5>(hookedAddress, Hook());
		}

		// hkbClipGenerator::Deactivate
		{
			static std::uintptr_t hookedAddress = hkbClipGenerator::Deactivate.address() + 0x1A;

			struct Hook : Xbyak::CodeGenerator
			{
				Hook()
				{
					Xbyak::Label hookLabel;
					Xbyak::Label retnLabel;

					mov(rdx, r14);	// r14 = hkbContext*
					call(ptr[rip + hookLabel]);

					jmp(ptr[rip + retnLabel]);

					L(hookLabel), dq(reinterpret_cast<std::uintptr_t>(&hkbClipGenerator::ResetIgnoreStartTime_Hook));
					L(retnLabel), dq(hookedAddress + 5);
				}
			};

			hkbClipGenerator::ResetIgnoreStartTime = utils::WriteBranchTrampoline<5>(hookedAddress, Hook());
		}

		// Character::ProcessMotionData
		{
			static std::uintptr_t translation1HookedAddress = Character::ProcessMotionData.address() + (!REL::Module::IsAE() ? 0x28D : 0x298);
			static std::uintptr_t translation2HookedAddress = Character::ProcessMotionData.address() + (!REL::Module::IsAE() ? 0x2A1 : 0x2AA);
			static std::uintptr_t rotation1HookedAddress = Character::ProcessMotionData.address() + (!REL::Module::IsAE() ? 0x355 : 0x35C);
			static std::uintptr_t rotation2HookedAddress = Character::ProcessMotionData.address() + (!REL::Module::IsAE() ? 0x368 : 0x36D);

			struct Hook : Xbyak::CodeGenerator
			{
				Hook(void* a_func)
				{
					Xbyak::Label hookLabel;

					sub(rsp, 0x28);

					mov(ptr[rsp + 0x20], r13);	// r13 = Character*
					mov(r9, rbx);				// SE: rbx = BSFixedString*
					if (REL::Module::IsAE())
					{
						sub(r9, 0x14);			// AE: rbx - 0x14 = BSFixedString*
					}
					call(ptr[rip + hookLabel]);

					add(rsp, 0x28);

					ret();

					L(hookLabel), dq(reinterpret_cast<std::uintptr_t>(a_func));
				}
			};

			// Translation
			utils::WriteCallTrampoline<5>(translation1HookedAddress, Hook(&MotionDataContainer::ProcessTranslationData_Hook));
			utils::WriteCallTrampoline<5>(translation2HookedAddress, Hook(&MotionDataContainer::ProcessTranslationData_Hook));

			//// Rotation
			utils::WriteCallTrampoline<5>(rotation1HookedAddress, Hook(&MotionDataContainer::ProcessRotationData_Hook));
			utils::WriteCallTrampoline<5>(rotation2HookedAddress, Hook(&MotionDataContainer::ProcessRotationData_Hook));
		}
	}
}
