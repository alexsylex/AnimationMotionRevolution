#include "Hooks.h"

#include "AnimMotionHandler.h"

#include "utils/Logger.h"

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
}
