#include "Hooks.h"

#include "AnimMotionHandler.h"

#include "utils/Logger.h"

#include "RE/H/hkpCharacterMovementUtil.h"
#include "RE/H/hkVector4.h"

#include "RE/RTTI.h"

namespace hooks
{
	// Container for each character instance that is playing an animation with custom motion data
	class CharacterClipAnimMotionMap
	{
	public:
		static CharacterClipAnimMotionMap* GetSingleton()
		{
			static CharacterClipAnimMotionMap singleton;

			return &singleton;
		}

		template <typename StringT>
		void Add(const RE::hkbCharacter* a_hkbCharacter, const StringT& a_clipName, const AnimMotionData& a_animMotionData)
		{
			std::string clipName{ a_clipName.c_str() };

			try {
				data[a_hkbCharacter][clipName] = a_animMotionData;
			} catch (const std::exception& e) {
				logger::warn("Exception thrown: {}, clearing to avoid possible memory overflow", e.what());
				logger::flush();

				data.clear();
			}
		}

		template <typename StringT>
		AnimMotionData* Get(const RE::hkbCharacter* a_hkbCharacter, const StringT& a_clipName)
		{
			std::string clipName{ a_clipName.c_str() };

			if (data.contains(a_hkbCharacter) && data[a_hkbCharacter].contains(clipName)) {
				return &data[a_hkbCharacter][clipName];
			} else {
				return nullptr;
			}
		}

		template <typename StringT>
		void Remove(const RE::hkbCharacter* a_hkbCharacter, const StringT& a_clipName)
		{
			std::string clipName{ a_clipName.c_str() };

			if (data.contains(a_hkbCharacter) && data[a_hkbCharacter].contains(clipName)) {
				if (data[a_hkbCharacter].size() == 1) {
					data.erase(a_hkbCharacter);
				} else {
					data[a_hkbCharacter].erase(clipName);
				}
			}
		}

		mutable RE::BSSpinLock lock;

	private:
		std::map<const RE::hkbCharacter*, std::map<std::string, AnimMotionData>> data;
	};

	// Called when animations are activated by clip generators
	std::uint32_t hkbClipGenerator::ComputeStartTime_Hook(const RE::hkbClipGenerator* a_this, const RE::hkbContext* a_hkbContext)
	{
		auto characterClipAnimMotionMap = CharacterClipAnimMotionMap::GetSingleton();

		RE::BSSpinLockGuard lockguard(characterClipAnimMotionMap->lock);

		const RE::hkaAnimation* boundAnimation = GetBoundAnimation(a_this);

		if (boundAnimation)
		{
			const RE::hkbCharacter* hkbCharacter = a_hkbContext ? a_hkbContext->character : nullptr;

			if (hkbCharacter)
			{
				AnimMotionData* animMotionData = characterClipAnimMotionMap->Get(hkbCharacter, a_this->name);

				// Animation activation is multithreaded, therefore I need to keep track of the number of times
				// they are activated
				if (animMotionData && animMotionData->animation == boundAnimation)
				{
					animMotionData->activeCount++;
				}
				else
				{
					Translation* translation = nullptr;
					Rotation* rotation = nullptr;

					for (const RE::hkaAnnotationTrack& annotationTrack : boundAnimation->annotationTracks)
					{
						for (const RE::hkaAnnotationTrack::Annotation& annotation : annotationTrack.annotations)
						{
							auto dataParsed = ParseAnnotation(annotation);

							translation = std::get_if<Translation>(&dataParsed);

							if (translation)
							{
								if (animMotionData && animMotionData->animation == boundAnimation)
								{
									animMotionData->Add(translation);
								}
								else
								{
									characterClipAnimMotionMap->Add(hkbCharacter, a_this->name, AnimMotionData{ boundAnimation, translation });

									if (!animMotionData)
									{
										animMotionData = characterClipAnimMotionMap->Get(hkbCharacter, a_this->name);
									}
								}
							}
							else
							{
								rotation = std::get_if<Rotation>(&dataParsed);

								if (rotation)
								{
									if (animMotionData && animMotionData->animation == boundAnimation)
									{
										animMotionData->Add(rotation);
									}
									else
									{
										characterClipAnimMotionMap->Add(hkbCharacter, a_this->name, AnimMotionData{ boundAnimation, rotation });

										if (!animMotionData)
										{
											animMotionData = characterClipAnimMotionMap->Get(hkbCharacter, a_this->name);
										}
									}
								}
							}
						}

						if (animMotionData)
						{
							animMotionData->SortListsByTime();

							if (!animMotionData->translationList.empty() && animMotionData->translationList.back().time != boundAnimation->duration)
							{
								logger::warn("Animation={} of hkbCharacter=0x{:08x} ends at {}, while custom translation ends at {}",
									a_this->animationName.c_str(), reinterpret_cast<std::uint64_t>(hkbCharacter),
									boundAnimation->duration, animMotionData->translationList.back().time);
							}

							if (!animMotionData->rotationList.empty() && animMotionData->rotationList.back().time != boundAnimation->duration)
							{
								logger::warn("Animation={} of hkbCharacter=0x{:08x} ends at {}, while custom rotation ends at {}",
									a_this->animationName.c_str(), reinterpret_cast<std::uint64_t>(hkbCharacter),
									boundAnimation->duration, animMotionData->rotationList.back().time);
							}

							// Support only for annotations in the same track, quit the loop when found
							break;
						}
					}
				}
			}
		}

		return ComputeStartTime(a_this);
	}

	// Called when animations are deactivated by clip generators
	void hkbClipGenerator::ResetIgnoreStartTime_Hook(const RE::hkbClipGenerator* a_this, const RE::hkbContext* a_hkbContext)
	{
		auto characterClipAnimMotionMap = CharacterClipAnimMotionMap::GetSingleton();

		RE::BSSpinLockGuard lockguard(characterClipAnimMotionMap->lock);

		const RE::hkaAnimation* boundAnimation = GetBoundAnimation(a_this);

		if (boundAnimation)
		{
			const RE::hkbCharacter* hkbCharacter = a_hkbContext ? a_hkbContext->character : nullptr;

			if (hkbCharacter)
			{
				AnimMotionData* animMotionData = characterClipAnimMotionMap->Get(hkbCharacter, a_this->name);

				// Animation deactivation is also multithreaded, so keep track of the number of
				// activated times left
				if (animMotionData && animMotionData->animation == boundAnimation)
				{
					animMotionData->activeCount--;

					// Erase from the list when deactivated same times as activated
					if (!animMotionData->activeCount)
					{
						characterClipAnimMotionMap->Remove(hkbCharacter, a_this->name);
					}
				}
			}
		}

		ResetIgnoreStartTime(a_this);
	}

	void MotionDataContainer::ProcessTranslationData_Hook(RE::MotionDataContainer* a_this, float a_motionTime,
														  RE::NiPoint3& a_translation, const RE::BSFixedString* a_clipName,
														  RE::Character* a_character)
	{
		auto characterClipAnimMotionMap = CharacterClipAnimMotionMap::GetSingleton();

		RE::BSSpinLockGuard lockguard(characterClipAnimMotionMap->lock);

		RE::hkbCharacter* hkbCharacter = GethkbCharacter(a_character);

		AnimMotionData* animMotionData = characterClipAnimMotionMap->Get(hkbCharacter, *a_clipName);

		std::vector<Translation>* customTranslationList = animMotionData ? &animMotionData->translationList : nullptr;

		bool hasCustomMotionList = customTranslationList && !customTranslationList->empty();

		if (hasCustomMotionList)
		{
			float endMotionTime = customTranslationList->back().time;

			float curMotionTime = (a_motionTime > endMotionTime) ? endMotionTime : a_motionTime;

			auto segCount = static_cast<std::uint32_t>(customTranslationList->size());

			for (std::uint32_t segIndex = 1; segIndex <= segCount; segIndex++)
			{
				float curSegMotionTime = customTranslationList->at(segIndex - 1).time;

				if (curMotionTime <= curSegMotionTime)
				{
					std::uint32_t prevSegIndex = segIndex - 1;
					float segProgress = 1.0f;

					float prevSegMotionTime = prevSegIndex ? customTranslationList->at(prevSegIndex - 1).time : 0.0f;

					float curSegMotionDuration = curSegMotionTime - prevSegMotionTime;
					if (curSegMotionDuration > std::numeric_limits<float>::epsilon())
					{
						segProgress = (curMotionTime - prevSegMotionTime) / curSegMotionDuration;
					}

					const RE::NiPoint3& curSegTranslation = customTranslationList->at(segIndex - 1).delta;

					const RE::NiPoint3& prevSegTranslation = prevSegIndex ?
																   customTranslationList->at(prevSegIndex - 1).delta :
																   RE::NiPoint3{ 0.0f, 0.0f, 0.0f };

					a_translation = (curSegTranslation * segProgress + prevSegTranslation * (1.0f - segProgress));

					auto charController = a_character->GetCharController();
					auto charStateOnGround = reinterpret_cast<RE::bhkCharacterStateOnGround*>(charController->context.stateManager->registeredState[RE::hkpCharacterStateType::kOnGround]);

					charStateOnGround->unk10 = a_translation.z == 0.0;


					return;
				}
			}
		}
		else
		{
			// The game checks this in the original code, so we do
			bool hasVanillaMotionList = a_this->translationSegCount > static_cast<std::uint32_t>(a_this->IsTranslationDataAligned());

			if (hasVanillaMotionList)
			{
				ProcessTranslationData(&a_this->translationDataPtr, a_motionTime, a_translation);

				return;
			}
		}

		a_translation = RE::NiPoint3{ 0.0f, 0.0f, 0.0f };
	}

	void MotionDataContainer::ProcessRotationData_Hook(RE::MotionDataContainer* a_this, float a_motionTime,
													   RE::NiQuaternion& a_rotation, const RE::BSFixedString* a_clipName,
													   RE::Character* a_character)
	{
		auto characterClipAnimMotionMap = CharacterClipAnimMotionMap::GetSingleton();
		RE::BSSpinLockGuard lockguard(characterClipAnimMotionMap->lock);

		RE::hkbCharacter* hkbCharacter = GethkbCharacter(a_character);

		auto characterController = a_character->GetCharController();

		AnimMotionData* animMotionData = characterClipAnimMotionMap->Get(hkbCharacter, *a_clipName);

		std::vector<Rotation>* customRotationList = animMotionData ? &animMotionData->rotationList : nullptr;

		bool hasCustomMotionList = customRotationList && !customRotationList->empty();

		if (hasCustomMotionList)
		{
			float endMotionTime = customRotationList->back().time;

			float curMotionTime = (a_motionTime > endMotionTime) ? endMotionTime : a_motionTime;

			for (std::uint32_t segIndex = 1; segIndex <= customRotationList->size(); segIndex++)
			{
				float curSegMotionTime = customRotationList->at(segIndex - 1).time;

				if (curMotionTime <= curSegMotionTime)
				{
					std::uint32_t prevSegIndex = segIndex - 1;
					float segProgress = 1.0f;

					float prevSegMotionTime = prevSegIndex ? customRotationList->at(prevSegIndex - 1).time : 0.0f;

					float curSegMotionDuration = curSegMotionTime - prevSegMotionTime;
					if (curSegMotionDuration > std::numeric_limits<float>::epsilon())
					{
						segProgress = (curMotionTime - prevSegMotionTime) / curSegMotionDuration;
					}
					const RE::NiQuaternion& curSegRotation = customRotationList->at(segIndex - 1).delta;
					const RE::NiQuaternion& prevSegRotation = prevSegIndex ?
																	customRotationList->at(prevSegIndex - 1).delta :
																	RE::NiQuaternion{ 1.0f, 0.0f, 0.0f, 0.0f };

					InterpolateRotation(a_rotation, segProgress, prevSegRotation, curSegRotation);

					return;
				}
			}
		}
		else
		{
			// The game checks this in the original code, so we do
			bool hasVanillaMotionList = a_this->rotationSegCount > static_cast<std::uint32_t>(a_this->IsRotationDataAligned());

			if (hasVanillaMotionList)
			{
				ProcessRotationData(&a_this->rotationDataPtr, a_motionTime, a_rotation);

				return;
			}
		}

		a_rotation = RE::NiQuaternion{ 1.0f, 0.0f, 0.0f, 0.0f };
	}

	RE::hkpCharacterStateType hkpCharacterContext::GetCharacterState_Hook(RE::hkpCharacterContext* a_this)
	{
		auto charStateOnGround = reinterpret_cast<RE::bhkCharacterStateOnGround*>(a_this->stateManager->registeredState[RE::hkpCharacterStateType::kOnGround]);

		return charStateOnGround->unk10 ? a_this->currentState : RE::hkpCharacterStateType::kSwimming;
	}

	void SimulateStatePhysics(RE::bhkCharacterStateOnGround* a_this, RE::bhkCharacterController* a_characterController)
	{
		RE::Character* character = nullptr;
		for (RE::BSTEventSink<RE::bhkCharacterMoveFinishEvent>* sink : a_characterController->sinks)
		{
			character = skyrim_cast<RE::Character*>(sink);
			if (character)
			{
				break;
			}
			else
			{
				character = skyrim_cast<RE::PlayerCharacter*>(sink);
				if (character)
				{
					break;
				}
			}
		}

		RE::BSAnimationGraphManagerPtr animGraphManager;

		if (character && character->GetAnimationGraphManager(animGraphManager))
		{
			std::uint32_t activeGraph = animGraphManager->GetRuntimeData().activeGraph;

			RE::BShkbAnimationGraph* animGraph = animGraphManager->graphs[activeGraph].get();
			RE::BSTEventSource<RE::BSAnimationGraphEvent>* animGraphEventSource = animGraph;

			RE::BSAnimationGraphEvent event{};

			animGraphEventSource->SendEvent(&event);
		}

		RE::bhkWorld* world = a_characterController->GetHavokWorld();

		RE::hkVector4 position = a_characterController->GetPosition();

		a_characterController->fallStartHeight = position.z * 69.991249;

		std::uint32_t collisionFilterInfo;
		a_characterController->GetCollisionFilterInfo(collisionFilterInfo);

		RE::bhkPickData pickData;
		pickData.rayInput.from = position;
		pickData.rayInput.to = position - RE::hkVector4{ 0.0F, 0.0F, 0.5F, 0.0F };
		pickData.rayInput.filterInfo = collisionFilterInfo;

		world->PickObject(pickData);

		if (pickData.rayOutput.rootCollidable)
		{
			if (pickData.rayOutput.normal.z < a_characterController->unk300)
			{
				a_characterController->flags.set(RE::CHARACTER_FLAGS::kSupport);
			}
		}
		else
		{
			a_characterController->wantState = RE::hkpCharacterStateType::kInAir;
			a_characterController->flags.reset(RE::CHARACTER_FLAGS::kSupport);
		}

		RE::hkVector4 desiredVelocity = a_characterController->velocityMod;

		desiredVelocity.x = a_characterController->velocityMod.y;
		desiredVelocity.y = a_characterController->velocityMod.x;
		
		RE::hkpCharacterMovementUtil::hkpMovementUtilInput movementUtil;
		movementUtil.forward = a_characterController->forwardVec;
		movementUtil.up = RE::hkVector4{ 0.0F, 0.0F, 1.0F, 0.0F };
		movementUtil.surfaceNormal = a_characterController->supportNorm;
		movementUtil.currentVelocity = a_characterController->outVelocity;
		movementUtil.desiredVelocity = desiredVelocity;
		movementUtil.surfaceVelocity = a_characterController->surfaceInfo.surfaceVelocity;
		movementUtil.gain = 1.0F;
		movementUtil.maxVelocityDelta = 500.0F;

		RE::hkpCharacterMovementUtil::CalculateMovement(movementUtil, a_characterController->outVelocity);

		a_characterController->outVelocity += a_characterController->surfaceInfo.surfaceVelocity;

		float gravity = world->GetGravity().quad.m128_f32[0];

		a_characterController->outVelocity += gravity * a_characterController->stepInfo.deltaTime;

		a_characterController->SetWantedState();

		RE::hkpCharacterStateType stateType = a_characterController->context.currentState;

		if (stateType != RE::hkpCharacterStateType::kOnGround)
		{
			auto characterState = static_cast<RE::bhkCharacterState*>(a_characterController->context.stateManager->registeredState[stateType]);

			characterState->SimulateStatePhysics(a_characterController);
		}
	}
}