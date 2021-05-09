#include "Hooks.h"

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
		float animationDuration;
	};

	std::map<RE::hkbCharacter*, AnimMotionData>	characterMotionMap;
	std::map<DWORD, AnimMotionData>				threadIdMotionMap;

	// static
	uint32_t __fastcall hkbClipGenerator::sub_140A0F480_Hook(RE::hkbClipGenerator* a_this)
	{
		RE::hkbCharacter* character = GethkbContext()->character;

		if(characterMotionMap.contains(character))
		{
			characterMotionMap.erase(character);
		}

		if(a_this->binding && a_this->binding->animation)
		{
			for(RE::hkaAnnotationTrack& annotationTrack : a_this->binding->animation->annotationTracks)
			{
				for(RE::hkaAnnotationTrack::Annotation& annotation : annotationTrack.annotations)
				{
					MotionData motion;
					if(motion.ParseFromAnnotation(annotation))
					{
						if(!characterMotionMap.contains(character))
						{
							AnimMotionData data;

							data.motionList.push_back(motion);
							data.animationDuration = a_this->binding->animation->duration;
							characterMotionMap[character] = data;
						}
						else
						{
							characterMotionMap[character].motionList.push_back(motion);
						}

						logger::info("[hkbClipGenerator] Added 0x{:x} {} = {} {} {} {}",
										reinterpret_cast<uint64_t>(character), a_this->animationName.c_str(),
										motion.time, motion.offset.x, motion.offset.y, motion.offset.z);
						logger::flush();
					}
				}
			}
		}

		return sub_140A0F480(a_this);
	}

	// static
	bool __fastcall BShkbAnimationGraph::sub_140AF0360_Hook(RE::BShkbAnimationGraph* a_this, float a_fVal, uint64_t a_u64Val)
	{
		RE::hkbCharacter* character = &a_this->characterInstance;

		if(characterMotionMap.contains(character))
		{
			DWORD threadId = GetCurrentThreadId();
			if(!threadIdMotionMap.contains(threadId))
			{
				threadIdMotionMap[threadId] = characterMotionMap[character];

				logger::info("[BShkbAnimationGraph] Added threadIdMotionMap and characterMotionMap");
			}
		}

		return sub_140AF0360(a_this, a_fVal, a_u64Val); 
	}

	// static
	uint32_t __fastcall BSMotionDataContainer::ApplyMotionData_Hook(RE::BSMotionDataContainer* a_this, float a_motionTime, RE::NiPoint3* a_pos)
	{
		DWORD threadId = GetCurrentThreadId();

		if(threadIdMotionMap.contains(threadId))
		{
			std::vector<MotionData>& motionList = threadIdMotionMap[threadId].motionList;

			float startMotionTime = motionList.front().time;
			float endMotionTime = motionList.back().time;

			float curMotionTime = (a_motionTime > endMotionTime)? endMotionTime :
				(a_motionTime < startMotionTime)? startMotionTime : a_motionTime;

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
						RE::NiPoint3{ 0, 0, 0 };

					*a_pos = curSegMotionOffset * segProgress + prevSegMotionOffset * (1.0f - segProgress);

					if(a_motionTime > threadIdMotionMap[threadId].animationDuration)
					{
						logger::info("[BSMotionDataContainer] Applying custom motion data!");
						for(const MotionData& motion : motionList)
						{
							logger::info("[BSMotionDataContainer] {} {} {} {}",
										 motion.time, motion.offset.x, motion.offset.y, motion.offset.z);
						}

						// Should be erase, this thread ID map, but not all threads reach this point...
						// TODO: Find out why
						//threadIdMotionMap.erase(threadId);
						threadIdMotionMap.clear();

						logger::info("[BSMotionDataContainer] Erasing maps");
						logger::flush();
					}

					return 0x3F800000;
				}
			}

			// TODO: Find out how the return value is computed (although it seems not being used)
			return 0x3F800000;
		}
		else
		{
			logger::info("[BSMotionDataContainer] Applying normal motion data...");
			logger::flush();

			return ApplyMotionData(a_this, a_motionTime, a_pos);
		}
	}
}
