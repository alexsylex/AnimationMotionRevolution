#include "Hooks.h"

#include <numbers>
#include <limits>
#include <charconv>

#include "Logger.h"

namespace AMR
{
	constexpr std::string_view animmotion_prefix = "animmotion ";
	constexpr std::string_view animrotation_prefix = "animrotation ";

	struct TranslationData
	{
		bool ParseFromAnnotation(RE::hkaAnnotationTrack::Annotation& a_annotation)
		{
			std::string_view text{ a_annotation.text.c_str() };

			if(text.starts_with(animmotion_prefix))
			{
				time = a_annotation.time;

				std::string_view start = text.substr(animmotion_prefix.size());
				std::string_view end = start.substr(start.find(' ') + 1);

				std::from_chars(start.data(), end.data(), pos.x);

				start = end;
				end = start.substr(start.find(' ') + 1);

				std::from_chars(start.data(), end.data(), pos.y);

				start = end;
				end = start.substr(start.size());

				std::from_chars(start.data(), end.data(), pos.z);

				return true;
			}

			return false;
		}

		float time;
		RE::NiPoint3 pos;
	};

	struct RotationData
	{
		bool ParseFromAnnotation(RE::hkaAnnotationTrack::Annotation& a_annotation)
		{
			std::string_view text{ a_annotation.text.c_str() };

			if(text.starts_with(animrotation_prefix))
			{
				time = a_annotation.time;

				std::string_view start = text.substr(animrotation_prefix.size());
				std::string_view end = start.substr(start.find(' ') + 1);
				start = end;
				end = start.substr(start.size());

				float yawDegrees;
				std::from_chars(start.data(), end.data(), yawDegrees);

				float roll = 0.0f;
				float pitch = 0.0f;
				float yaw = -yawDegrees * std::numbers::pi_v<float> / 180.0f;

				rotation.w = std::cos(roll / 2) * std::cos(pitch / 2) * std::cos(yaw / 2) + std::sin(roll / 2) * std::sin(pitch / 2) * std::sin(yaw / 2);
				rotation.x = std::sin(roll / 2) * std::cos(pitch / 2) * std::cos(yaw / 2) - std::cos(roll / 2) * std::sin(pitch / 2) * std::sin(yaw / 2);
				rotation.y = std::cos(roll / 2) * std::sin(pitch / 2) * std::cos(yaw / 2) + std::sin(roll / 2) * std::cos(pitch / 2) * std::sin(yaw / 2);
				rotation.z = std::cos(roll / 2) * std::cos(pitch / 2) * std::sin(yaw / 2) - std::sin(roll / 2) * std::sin(pitch / 2) * std::cos(yaw / 2);

				return true;
			}

			return false;
		}

		float time;
		RE::NiQuaternion rotation;
	};

	struct AnimMotionData
	{
		RE::hkaAnimation* animation;
		std::vector<TranslationData> translationList;
		std::vector<RotationData> rotationList;
		int activeCount;
	};

	std::unordered_map<RE::hkbCharacter*, AnimMotionData> characterMotionMap;

	// static
	std::uint32_t hkbClipGenerator::sub_140A0F480_Hook(RE::hkbClipGenerator* a_this)
	{
		static auto GethkbContext = []() -> RE::hkbContext*
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
		};

		if(a_this->binding && a_this->binding->animation)
		{
			if(RE::hkbContext* context = GethkbContext())
			{
				RE::hkbCharacter* character = context->character;

				if(characterMotionMap.contains(character) && (characterMotionMap[character].animation == a_this->binding->animation.get()))
				{
					characterMotionMap[character].activeCount++;
				}
				else
				{
					bool isAnimMotionFound = false;
					for(RE::hkaAnnotationTrack& annotationTrack : a_this->binding->animation->annotationTracks)
					{
						for(RE::hkaAnnotationTrack::Annotation& annotation : annotationTrack.annotations)
						{
							TranslationData translation;
							RotationData rotation;
							if(translation.ParseFromAnnotation(annotation))
							{
								isAnimMotionFound = true;

								if(!characterMotionMap.contains(character) ||
								   (characterMotionMap[character].animation != a_this->binding->animation.get()))
								{
									if((characterMotionMap[character].animation != a_this->binding->animation.get()))
									{
										characterMotionMap.erase(character);
									}

									AnimMotionData data;
									data.translationList.push_back(translation);
									data.animation = a_this->binding->animation.get();
									data.activeCount = 1;

									characterMotionMap[character] = data;
								}
								else
								{
									characterMotionMap[character].translationList.push_back(translation);
								}
							}
							else if(rotation.ParseFromAnnotation(annotation))
							{
								isAnimMotionFound = true;

								if(!characterMotionMap.contains(character) ||
								   (characterMotionMap[character].animation != a_this->binding->animation.get()))
								{
									if((characterMotionMap[character].animation != a_this->binding->animation.get()))
									{
										characterMotionMap.erase(character);
									}

									AnimMotionData data;
									data.rotationList.push_back(rotation);
									data.animation = a_this->binding->animation.get();
									data.activeCount = 1;

									characterMotionMap[character] = data;
								}
								else
								{
									characterMotionMap[character].rotationList.push_back(rotation);
								}
							}
						}

						if(isAnimMotionFound)
						{
							std::sort(characterMotionMap[character].translationList.begin(), characterMotionMap[character].translationList.end(),
									  [](const TranslationData& a_lhs, const TranslationData& a_rhs) -> bool
									  {
										  return a_lhs.time < a_rhs.time;
									  });
							break;
						}
					}
				}
			}
		}

		return sub_140A0F480(a_this);
	}

	// static
	void hkbClipGenerator::sub_140A0F610_Hook(RE::hkbClipGenerator* a_this)
	{
		static auto GethkbContext = []() -> RE::hkbContext*
		{
			struct GethkbContext : Xbyak::CodeGenerator
			{
				GethkbContext()
				{
					mov(rax, r14); // r14 = hkbContext*
					ret();
				}
			} gethkbContext;

			return gethkbContext.getCode<RE::hkbContext* (*)()>()();
		};

		if(RE::hkbContext* context = GethkbContext())
		{
			RE::hkbCharacter* character = context->character;

			if(characterMotionMap.contains(character) && (characterMotionMap[character].animation == a_this->binding->animation.get()))
			{
				characterMotionMap[character].activeCount--;

				if(!characterMotionMap[character].activeCount)
				{
					characterMotionMap.erase(character);
				}
			}
		}
	}

	// static
	void MotionDataContainer::sub_1404DD5A0_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiPoint3& a_translation)
	{
		static auto GethkbCharacter = []() -> RE::hkbCharacter*
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
		};

		RE::hkbCharacter* character = GethkbCharacter();

		if(characterMotionMap.contains(character))
		{
			logger::info("AMR");
			logger::flush();

			std::vector<TranslationData>& motionList = characterMotionMap[character].translationList;

			float endMotionTime = motionList.back().time;

			float curMotionTime = (a_motionTime > endMotionTime)? endMotionTime : a_motionTime;

			for(unsigned int segIndex = 1; segIndex <= motionList.size(); segIndex++)
			{
				float curSegMotionTime = motionList.at(segIndex - 1).time;

				if(curMotionTime <= curSegMotionTime)
				{
					int prevSegIndex = segIndex - 1;
					float segProgress = 1.0f;

					float prevSegMotionTime = prevSegIndex? motionList.at(prevSegIndex - 1).time : 0.0f;

					float curSegMotionDuration = curSegMotionTime - prevSegMotionTime;
					if(curSegMotionDuration > std::numeric_limits<float>::epsilon())
					{
						segProgress = (curMotionTime - prevSegMotionTime) / curSegMotionDuration;
					}
					const RE::NiPoint3& curSegTranslation = motionList.at(segIndex - 1).pos;
					const RE::NiPoint3& prevSegTranslation = prevSegIndex?
						motionList.at(prevSegIndex - 1).pos : RE::NiPoint3{ 0.0f, 0.0f, 0.0f };

					a_translation = curSegTranslation * segProgress + prevSegTranslation * (1.0f - segProgress);

					return;
				}
			}
		}
		else if(a_this->translationSegCount > static_cast<unsigned int>(a_this->IsTranslationDataAligned()))
		{
			logger::info("vanilla");
			logger::flush();

			ProcessTranslationData(&a_this->translationDataPtr, a_motionTime, a_translation);

			return;
		}

		a_translation = RE::NiPoint3{ 0.0f, 0.0f, 0.0f };
	}

	// static
	void MotionDataContainer::sub_1404DD5F0_Hook(RE::MotionDataContainer* a_this, float a_motionTime, RE::NiQuaternion& a_rotation)
	{
		static auto GethkbCharacter = []() -> RE::hkbCharacter*
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
		};
		
		RE::hkbCharacter* character = GethkbCharacter();
		
		if(characterMotionMap.contains(character))
		{
			std::vector<RotationData>& motionList = characterMotionMap[character].rotationList;
		
			float endMotionTime = motionList.back().time;
		
			float curMotionTime = (a_motionTime > endMotionTime)? endMotionTime : a_motionTime;
		
			for(unsigned int segIndex = 1; segIndex <= motionList.size(); segIndex++)
			{
				float curSegMotionTime = motionList.at(segIndex - 1).time;
		
				if(curMotionTime <= curSegMotionTime)
				{
					int prevSegIndex = segIndex - 1;
					float segProgress = 1.0f;
		
					float prevSegMotionTime = prevSegIndex? motionList.at(prevSegIndex - 1).time : 0.0f;
		
					float curSegMotionDuration = curSegMotionTime - prevSegMotionTime;
					if(curSegMotionDuration > std::numeric_limits<float>::epsilon())
					{
						segProgress = (curMotionTime - prevSegMotionTime) / curSegMotionDuration;
					}
					const RE::NiQuaternion& curSegRotation = motionList.at(segIndex - 1).rotation;
					const RE::NiQuaternion& prevSegRotation = prevSegIndex?
						motionList.at(prevSegIndex - 1).rotation :
						RE::NiQuaternion{ 1.0f, 0.0f, 0.0f, 0.0f };

					InterpolateRotation(a_rotation, segProgress, prevSegRotation, curSegRotation);

					return;
				}
			}
		}
		else if(a_this->rotationSegCount > static_cast<unsigned int>(a_this->IsRotationDataAligned()))
		{
			ProcessRotationData(&a_this->rotationDataPtr, a_motionTime, a_rotation);

			return;
		}

		a_rotation = RE::NiQuaternion{ 1.0f, 0.0f, 0.0f, 0.0f };
	}
}
