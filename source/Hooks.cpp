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

		struct Translation
		{
			float time;
			RE::NiPoint3 translation;
		};

		struct Rotation
		{
			float time;
			RE::NiQuaternion rotation;
		};

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

	// Annotations that we are looking for can contain translation or rotation data
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
	std::map<const RE::hkbCharacter*, AnimMotionData> g_charAnimMotionMap;

	// Called when animations are activated by clip generators
	/* static */ std::uint32_t hkbClipGenerator::unk_A0F480_Hook(RE::hkbClipGenerator* a_this)
	{
		static auto GethkbContext = []() -> const RE::hkbContext*
		{
			struct GethkbContext : Xbyak::CodeGenerator
			{
				GethkbContext()
				{
					mov(rax, r15); // r15 = hkbContext*
					ret();
				}
			} gethkbContext;

			return gethkbContext.getCode<const RE::hkbContext* (*)()>()();
		};

		const RE::hkaAnimation* boundAnimation = GetBoundAnimation(a_this);

		if (boundAnimation)
		{
			const RE::hkbContext* hkbContext = GethkbContext();
			const RE::hkbCharacter* hkbCharacter = hkbContext? hkbContext->character : nullptr;

			if (hkbCharacter)
			{
				AnimMotionData* animMotionData = g_charAnimMotionMap.contains(hkbCharacter)?
													&g_charAnimMotionMap[hkbCharacter] : nullptr;

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
									g_charAnimMotionMap.insert_or_assign(hkbCharacter, AnimMotionData{ boundAnimation, translation });

									if (!animMotionData) 
									{
										animMotionData = &g_charAnimMotionMap[hkbCharacter];	
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
										g_charAnimMotionMap.insert_or_assign(hkbCharacter, AnimMotionData{ boundAnimation, rotation });

										if (!animMotionData) 
										{
											animMotionData = &g_charAnimMotionMap[hkbCharacter];
										}
									}
								}
							}
						}

						if (animMotionData) 
						{
							animMotionData->SortListsByTime();

							logger::debug("{}", hkbCharacter->name.c_str());

							// Support only for annotations in the same track, quit the loop when found
							break;
						}
					}
				}
			}
		}

		return unk_A0F480(a_this);
	}

	// Called when animations are deactivated by clip generators
	/* static */ void hkbClipGenerator::unk_A0F610_Hook(RE::hkbClipGenerator* a_this)
	{
		static auto GethkbContext = []() -> const RE::hkbContext*
		{
			struct GethkbContext : Xbyak::CodeGenerator
			{
				GethkbContext()
				{
					mov(rax, r14); // r14 = hkbContext*
					ret();
				}
			} gethkbContext;

			return gethkbContext.getCode<const RE::hkbContext* (*)()>()();
		};

		const RE::hkaAnimation* boundAnimation = GetBoundAnimation(a_this);

		if (boundAnimation)
		{
			const RE::hkbContext* hkbContext = GethkbContext();
			const RE::hkbCharacter* hkbCharacter = hkbContext? hkbContext->character : nullptr;

			if (hkbCharacter)
			{			
				AnimMotionData* animMotionData = g_charAnimMotionMap.contains(hkbCharacter)?
													&g_charAnimMotionMap[hkbCharacter] : nullptr;

				// Animation deactivation is also multithreaded, so keep track of the number of 
				// activated times left
				if (animMotionData && animMotionData->animation == boundAnimation)
				{
					animMotionData->activeCount--;

					// Erase from the list when deactivated same times as activated
					if (!animMotionData->activeCount)
					{
						g_charAnimMotionMap.erase(hkbCharacter);
					}
				}
			}
		}
	
		unk_A0F610(a_this);
	}

	/* static */ RE::hkbCharacter* MotionDataContainer::GethkbCharacter(RE::Character* a_character)
	{
		RE::BSAnimationGraphManagerPtr animGraph;

		a_character->GetAnimationGraphManager(animGraph);

		if(animGraph && animGraph->graphs[animGraph->activeGraph]) 
		{
			return &animGraph->graphs[animGraph->activeGraph]->characterInstance;
		}

		return nullptr;
	};

	/* static */ void MotionDataContainer::unk_4DD5A0_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiPoint3& a_translation)
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

		RE::Character* character = GetCharacter();
		RE::hkbCharacter* hkbCharacter = GethkbCharacter(character);

		std::vector<AnimMotionData::Translation>* customMotionList = g_charAnimMotionMap.contains(hkbCharacter)?
														 &g_charAnimMotionMap[hkbCharacter].translationList : nullptr;

		bool hasCustomMotionList = customMotionList && !customMotionList->empty();

		// The game checks this in the original code, so we do
		bool hasVanillaMotionList = a_this->translationSegCount > static_cast<std::uint32_t>(a_this->IsTranslationDataAligned());

		if (hasCustomMotionList)
		{
			float endMotionTime = customMotionList->back().time;

			float curMotionTime = (a_motionTime > endMotionTime)? endMotionTime : a_motionTime;

			auto segCount = static_cast<std::uint32_t>(customMotionList->size());

			for (std::uint32_t segIndex = 1; segIndex <= segCount; segIndex++)
			{
				float curSegMotionTime = customMotionList->at(segIndex - 1).time;

				if (curMotionTime <= curSegMotionTime)
				{
					std::uint32_t prevSegIndex = segIndex - 1;
					float segProgress = 1.0f;

					float prevSegMotionTime = prevSegIndex? customMotionList->at(prevSegIndex - 1).time : 0.0f;

					float curSegMotionDuration = curSegMotionTime - prevSegMotionTime;
					if (curSegMotionDuration > std::numeric_limits<float>::epsilon())
					{
						segProgress = (curMotionTime - prevSegMotionTime) / curSegMotionDuration;
					}

					const RE::NiPoint3& curSegTranslation = customMotionList->at(segIndex - 1).translation;

					const RE::NiPoint3& prevSegTranslation = prevSegIndex? customMotionList->at(prevSegIndex - 1).translation : RE::NiPoint3{ 0.0f, 0.0f, 0.0f };

					a_translation = (curSegTranslation * segProgress + prevSegTranslation * (1.0f - segProgress));

					//RE::bhkCharacterController* characterController = character->GetCharController();
					//
					//if(a_translation.z)
					//{
					//	if(characterController->context.currentState == RE::hkpCharacterStateType::kOnGround)
					//	{
					//		characterController->wantState = RE::hkpCharacterStateType::kJumping;
					//		
					//	}
					//	else if(characterController->context.currentState == RE::hkpCharacterStateType::kFlying)
					//	{
					//		characterController->wantState = RE::hkpCharacterStateType::kInAir;
					//	}
					//	characterController->fallStartHeight = 0.0f;
					//}
					//
					//logger::info("State {}", static_cast<std::int32_t>(characterController->context.currentState));
					//logger::flush();

					return;
				}
			}
		} 
		else if (hasVanillaMotionList)
		{
			ProcessTranslationData(&a_this->translationDataPtr, a_motionTime, a_translation);

			return;
		}

		a_translation = RE::NiPoint3{ 0.0f, 0.0f, 0.0f };
	}

	/* static */ void MotionDataContainer::unk_4DD5F0_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiQuaternion& a_rotation)
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

		RE::hkbCharacter* hkbCharacter = GethkbCharacter(GetCharacter());

		std::vector<AnimMotionData::Rotation>* customMotionList = g_charAnimMotionMap.contains(hkbCharacter)?
															&g_charAnimMotionMap[hkbCharacter].rotationList : nullptr;

		bool hasCustomMotionList = customMotionList && !customMotionList->empty();

		// The game checks this in the original code, so we do
		bool hasVanillaMotionList = a_this->rotationSegCount > static_cast<std::uint32_t>(a_this->IsRotationDataAligned());

		if (hasCustomMotionList)
		{
			float endMotionTime = customMotionList->back().time;

			float curMotionTime = (a_motionTime > endMotionTime)? endMotionTime : a_motionTime;

			for (std::uint32_t segIndex = 1; segIndex <= customMotionList->size(); segIndex++)
			{
				float curSegMotionTime = customMotionList->at(segIndex - 1).time;

				if (curMotionTime <= curSegMotionTime)
				{
					std::uint32_t prevSegIndex = segIndex - 1;
					float segProgress = 1.0f;

					float prevSegMotionTime = prevSegIndex? customMotionList->at(prevSegIndex - 1).time : 0.0f;

					float curSegMotionDuration = curSegMotionTime - prevSegMotionTime;
					if (curSegMotionDuration > std::numeric_limits<float>::epsilon())
					{
						segProgress = (curMotionTime - prevSegMotionTime) / curSegMotionDuration;
					}
					const RE::NiQuaternion& curSegRotation = customMotionList->at(segIndex - 1).rotation;
					const RE::NiQuaternion& prevSegRotation = prevSegIndex?
						customMotionList->at(prevSegIndex - 1).rotation : RE::NiQuaternion{ 1.0f, 0.0f, 0.0f, 0.0f };

					InterpolateRotation(a_rotation, segProgress, prevSegRotation, curSegRotation);

					return;
				}
			}
		} 
		else if (hasVanillaMotionList) 
		{
			ProcessRotationData(&a_this->rotationDataPtr, a_motionTime, a_rotation);

			return;
		}

		a_rotation = RE::NiQuaternion{ 1.0f, 0.0f, 0.0f, 0.0f };
	}
}
