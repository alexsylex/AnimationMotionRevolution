#include "Hooks.h"

#include <numbers>
#include <limits>
#include <charconv>

#include "Logger.h"

namespace AMR
{
	class AnimMotionData
	{
	public:

		template<typename T>
		struct Motion
		{
			float time;
			T delta;
		};

		using Translation = Motion<RE::NiPoint3>;
		using Rotation = Motion<RE::NiQuaternion>;

		AnimMotionData() = default;

		AnimMotionData(const RE::hkaAnimation* a_animation, const Translation* a_translation) 
		: animation{ a_animation }, translationList{ *a_translation }
		{ }

		AnimMotionData(const RE::hkaAnimation* a_animation, const Rotation* a_rotation) 
		: animation{ a_animation }, rotationList{ *a_rotation }
		{ }

		void Add(const Translation* a_translation)
		{
			translationList.push_back(*a_translation);
		}

		void Add(const Rotation* a_rotation)
		{
			rotationList.push_back(*a_rotation);
		}

		void SortListsByTime()
		{
			if (!translationList.empty()) 
			{
				std::sort(translationList.begin(), translationList.end(),
					[](const Translation& a_lhs, const Translation& a_rhs) -> bool
					{
						return a_lhs.time < a_rhs.time;
					});
			}

			if (!rotationList.empty()) 
			{
				std::sort(rotationList.begin(), rotationList.end(),
					[](const Rotation& a_lhs, const Rotation& a_rhs) -> bool
					{
						return a_lhs.time < a_rhs.time;
					});
			}
		}

		const RE::hkaAnimation* animation = nullptr;
		std::vector<Translation> translationList;
		std::vector<Rotation> rotationList;
		std::int32_t activeCount = 1;
	};

	// Annotations we are looking for contain translation/rotation data
	static std::variant<std::monostate, AnimMotionData::Translation, AnimMotionData::Rotation>
		ParseAnnotation(const RE::hkaAnnotationTrack::Annotation& a_annotation)
	{
		constexpr std::string_view animmotionPrefix = "animmotion ";
		constexpr std::string_view animrotationPrefix = "animrotation ";

		std::string_view text{ a_annotation.text.c_str() };

		if (text.starts_with(animmotionPrefix))
		{
			RE::NiPoint3 translation;

			std::string_view start = text.substr(animmotionPrefix.size());
			std::string_view end = start.substr(start.find(' ') + 1);

			std::from_chars(start.data(), end.data(), translation.x);

			start = end;
			end = start.substr(start.find(' ') + 1);

			std::from_chars(start.data(), end.data(), translation.y);

			start = end;
			end = start.substr(start.size());

			std::from_chars(start.data(), end.data(), translation.z);

			return AnimMotionData::Translation{ a_annotation.time, translation };
		} 
		else if (text.starts_with(animrotationPrefix))
		{
			RE::NiQuaternion rotation;

			std::string_view start = text.substr(animrotationPrefix.size());
			std::string_view end = start.substr(start.size());

			float yawDegrees;
			std::from_chars(start.data(), end.data(), yawDegrees);

			float roll = 0.0f;
			float pitch = 0.0f;
			float yaw = yawDegrees * std::numbers::pi_v<float> / 180.0f;

			rotation.w = std::cos(roll / 2) * std::cos(pitch / 2) * std::cos(yaw / 2) + std::sin(roll / 2) * std::sin(pitch / 2) * std::sin(yaw / 2);
			rotation.x = std::sin(roll / 2) * std::cos(pitch / 2) * std::cos(yaw / 2) - std::cos(roll / 2) * std::sin(pitch / 2) * std::sin(yaw / 2);
			rotation.y = std::cos(roll / 2) * std::sin(pitch / 2) * std::cos(yaw / 2) + std::sin(roll / 2) * std::cos(pitch / 2) * std::sin(yaw / 2);
			rotation.z = std::cos(roll / 2) * std::cos(pitch / 2) * std::sin(yaw / 2) - std::sin(roll / 2) * std::sin(pitch / 2) * std::cos(yaw / 2);

			return AnimMotionData::Rotation{ a_annotation.time, rotation };
		}

		return { };
	}

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

			try 
			{
				data[a_hkbCharacter][clipName] = a_animMotionData;
			}
			catch (const std::exception& e) 
			{
				logger::warn("Exception thrown: {}, clearing to avoid possible memory overflow", e.what());
				logger::flush();

				data.clear();
			}
		}

		template<typename StringT>
		AnimMotionData* Get(const RE::hkbCharacter* a_hkbCharacter, const StringT& a_clipName)
		{
			std::string clipName{ a_clipName.c_str() };

			if (data.contains(a_hkbCharacter) && data[a_hkbCharacter].contains(clipName)) 
			{
				return &data[a_hkbCharacter][clipName];
			}
			else 
			{
				return nullptr;
			}
		}

		template <typename StringT>
		void Remove(const RE::hkbCharacter* a_hkbCharacter, const StringT& a_clipName)
		{
			std::string clipName{ a_clipName.c_str() };

			if (data.contains(a_hkbCharacter) && data[a_hkbCharacter].contains(clipName))
			{
				if (data[a_hkbCharacter].size() == 1) 
				{
					data.erase(a_hkbCharacter);
				} 
				else 
				{
					data[a_hkbCharacter].erase(clipName);
				}
			}
		}

	private:

		std::map<const RE::hkbCharacter*, std::map<std::string, AnimMotionData>> data;
	};

	// Called when animations are activated by clip generators
	std::uint32_t hkbClipGenerator::ComputeStartTime_Hook(const RE::hkbClipGenerator* a_this, const RE::hkbContext* a_hkbContext)
	{
		const RE::hkaAnimation* boundAnimation = GetBoundAnimation(a_this);

		if (boundAnimation)
		{
			const RE::hkbCharacter* hkbCharacter = a_hkbContext? a_hkbContext->character : nullptr;

			if (hkbCharacter)
			{
				auto characterClipAnimMotionMap = CharacterClipAnimMotionMap::GetSingleton();

				AnimMotionData* animMotionData = characterClipAnimMotionMap->Get(hkbCharacter, a_this->name);

				// Animation activation is multithreaded, therefore I need to keep track of the number of times
				// they are activated
				if (animMotionData && animMotionData->animation == boundAnimation)
				{
					animMotionData->activeCount++;
				}
				else
				{
					AnimMotionData::Translation* translation = nullptr;
					AnimMotionData::Rotation* rotation = nullptr;

					for (const RE::hkaAnnotationTrack& annotationTrack : boundAnimation->annotationTracks)
					{
						for (const RE::hkaAnnotationTrack::Annotation& annotation : annotationTrack.annotations)
						{
							auto dataParsed = ParseAnnotation(annotation);

							translation = std::get_if<AnimMotionData::Translation>(&dataParsed);

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
								rotation = std::get_if<AnimMotionData::Rotation>(&dataParsed);

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
								logger::warn(
									"Animation={} of hkbCharacter=0x{:08x} ends at {}, while custom rotation ends at {}",
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
		const RE::hkaAnimation* boundAnimation = GetBoundAnimation(a_this);

		if (boundAnimation)
		{
			const RE::hkbCharacter* hkbCharacter = a_hkbContext? a_hkbContext->character : nullptr;

			if (hkbCharacter)
			{
				auto characterClipAnimMotionMap = CharacterClipAnimMotionMap::GetSingleton();

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
											  RE::NiPoint3& a_translation, const RE::BSFixedString* a_clipName)
	{
		static auto GetCharacter = []() -> RE::Character*
		{
			struct GetCharacter : Xbyak::CodeGenerator
			{
				GetCharacter()
				{
					mov(rax, r13); // r13 = Character*
					ret();
				}
			} getCharacter;

			return getCharacter.getCode<RE::Character* (*)()>()();
		};

		logger::info("translation");
		logger::flush();

		return;

		RE::Character* character = GetCharacter();
		RE::hkbCharacter* hkbCharacter = GethkbCharacter(character);

		AnimMotionData* animMotionData = CharacterClipAnimMotionMap::GetSingleton()->Get(hkbCharacter, *a_clipName);

		std::vector<AnimMotionData::Translation>* customTranslationList = animMotionData?
			&animMotionData->translationList : nullptr;

		bool hasCustomMotionList = customTranslationList && !customTranslationList->empty();

		if (hasCustomMotionList)
		{
			float endMotionTime = customTranslationList->back().time;

			float curMotionTime = (a_motionTime > endMotionTime)? endMotionTime : a_motionTime;

			auto segCount = static_cast<std::uint32_t>(customTranslationList->size());

			for (std::uint32_t segIndex = 1; segIndex <= segCount; segIndex++)
			{
				float curSegMotionTime = customTranslationList->at(segIndex - 1).time;

				if (curMotionTime <= curSegMotionTime)
				{
					std::uint32_t prevSegIndex = segIndex - 1;
					float segProgress = 1.0f;

					float prevSegMotionTime = prevSegIndex? customTranslationList->at(prevSegIndex - 1).time : 0.0f;

					float curSegMotionDuration = curSegMotionTime - prevSegMotionTime;
					if (curSegMotionDuration > std::numeric_limits<float>::epsilon())
					{
						segProgress = (curMotionTime - prevSegMotionTime) / curSegMotionDuration;
					}

					const RE::NiPoint3& curSegTranslation = customTranslationList->at(segIndex - 1).delta;

					const RE::NiPoint3& prevSegTranslation = prevSegIndex? customTranslationList->at(prevSegIndex - 1).delta : RE::NiPoint3{ 0.0f, 0.0f, 0.0f };

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
		RE::NiQuaternion& a_rotation, const RE::BSFixedString* a_clipName)
	{
		static auto GetCharacter = []() -> RE::Character*
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

			return character;
		};

		logger::info("translation");
		logger::flush();

		return;


		RE::hkbCharacter* hkbCharacter = GethkbCharacter(GetCharacter());

		AnimMotionData* animMotionData = CharacterClipAnimMotionMap::GetSingleton()->Get(hkbCharacter, *a_clipName);

		std::vector<AnimMotionData::Rotation>* customRotationList = animMotionData ?
																		  &animMotionData->rotationList :
																		  nullptr;

		bool hasCustomMotionList = customRotationList && !customRotationList->empty();

		if (hasCustomMotionList)
		{
			float endMotionTime = customRotationList->back().time;

			float curMotionTime = (a_motionTime > endMotionTime)? endMotionTime : a_motionTime;

			for (std::uint32_t segIndex = 1; segIndex <= customRotationList->size(); segIndex++)
			{
				float curSegMotionTime = customRotationList->at(segIndex - 1).time;

				if (curMotionTime <= curSegMotionTime)
				{
					std::uint32_t prevSegIndex = segIndex - 1;
					float segProgress = 1.0f;

					float prevSegMotionTime = prevSegIndex? customRotationList->at(prevSegIndex - 1).time : 0.0f;

					float curSegMotionDuration = curSegMotionTime - prevSegMotionTime;
					if (curSegMotionDuration > std::numeric_limits<float>::epsilon())
					{
						segProgress = (curMotionTime - prevSegMotionTime) / curSegMotionDuration;
					}
					const RE::NiQuaternion& curSegRotation = customRotationList->at(segIndex - 1).delta;
					const RE::NiQuaternion& prevSegRotation = prevSegIndex?
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
