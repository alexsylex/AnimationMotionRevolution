#include "Hooks.h"

#include <tuple>
#include <limits>
#include <charconv>

#include "Logger.h"

namespace DMR
{
	constexpr std::string_view animmotion_prefix = "animmotion ";

	struct MotionData
	{
		bool ParseFromAnnotation(RE::hkaAnnotationTrack::Annotation& a_annotation)
		{
			std::string_view text{ a_annotation.text.c_str() };

			if(text.starts_with(animmotion_prefix))
			{
				time = a_annotation.time;

				std::string_view start = text.substr(animmotion_prefix.size());
				std::string_view end = start.substr(start.find(' ') + 1);

				std::from_chars(start.data(), end.data(), offset.x);

				start = end;
				end = start.substr(start.find(' ') + 1);

				std::from_chars(start.data(), end.data(), offset.y);

				start = end;
				end = start.substr(start.size());

				std::from_chars(start.data(), end.data(), offset.z);

				return true;
			}

			return false;
		}

		float time;
		RE::NiPoint3 offset;
	};

	struct AnimMotionData
	{
		std::vector<MotionData> motionList;
		RE::hkaAnimation* animation;
		int activeCount;
	};

	std::unordered_map<RE::hkbCharacter*, AnimMotionData> characterMotionMap;

	// static
	uint32_t __fastcall hkbClipGenerator::sub_140A0F480_Hook(RE::hkbClipGenerator* a_this)
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
			RE::hkbCharacter* character = GethkbContext()->character;

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
						MotionData motion;
						if(motion.ParseFromAnnotation(annotation))
						{
							isAnimMotionFound = true;

							if(!characterMotionMap.contains(character))
							{
								AnimMotionData data;
								data.motionList.push_back(motion);
								data.animation = a_this->binding->animation.get();
								data.activeCount = 1;

								characterMotionMap[character] = data;
							}
							else
							{
								characterMotionMap[character].motionList.push_back(motion);
							}
						}
					}

					if(isAnimMotionFound) break;
				}
			}

		}

		logger::flush();

		return sub_140A0F480(a_this);
	}

	// static
	void __fastcall hkbClipGenerator::sub_140A0F610_Hook(RE::hkbClipGenerator* a_this)
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

		RE::hkbCharacter* character = GethkbContext()->character;

		if(characterMotionMap.contains(character) && (characterMotionMap[character].animation == a_this->binding->animation.get()))
		{
			characterMotionMap[character].activeCount--;

			if(!characterMotionMap[character].activeCount)
			{ 
				characterMotionMap.erase(character);
			}
		}
	}

	// static
	void __fastcall BSMotionDataContainer::sub_1404DD5A0_Hook(RE::BSMotionDataContainer* a_this, float a_motionTime, RE::NiPoint3* a_pos)
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
			std::vector<MotionData>& motionList = characterMotionMap[character].motionList;

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
					const RE::NiPoint3& curSegMotionOffset = motionList.at(segIndex - 1).offset;
					const RE::NiPoint3& prevSegMotionOffset = prevSegIndex?
																motionList.at(prevSegIndex - 1).offset :
																RE::NiPoint3{ 0.0f, 0.0f, 0.0f };

					*a_pos = curSegMotionOffset * segProgress + prevSegMotionOffset * (1.0f - segProgress);

					return;
				}
			}
		}
		else if(a_this->segCount > static_cast<unsigned int>(a_this->IsDataAligned()))
		{
			ApplyMotionData(a_this, a_motionTime, a_pos);

			return;
		}
			
		*a_pos = RE::NiPoint3{ 0.0f, 0.0f, 0.0f };
	}
}
