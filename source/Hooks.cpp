#include "Hooks.h"
#include "Logger.h"

namespace DMR
{
	// static
	uint32_t __fastcall hkbClipGenerator::sub_140A0F480_Hook(RE::hkbClipGenerator* a_this)
	{
		auto ReadAnnotationTracks = [&a_this](RE::hkaAnimationBinding* a_binding)
		{
			if(a_binding->animation)
			{
				for(auto& annotationTrack : a_binding->animation.get()->annotationTracks)
				{
					for(auto& annotation : annotationTrack.annotations)
					{
						if(std::strlen(annotation.text.c_str()))
						{
							logger::info("Annotation found in {} =  {}", a_this->animationName.c_str(), annotation.text.c_str());
						}
					}
				}
			}
		};

		if(a_this->binding)
		{
			ReadAnnotationTracks(a_this->binding);
		}

		logger::info("[Thread ID = {}][hkbClipGenerator] Character pointer = 0x{:x}", 
					 GetCurrentThreadId(), reinterpret_cast<uint64_t>(GethkbContext()->character));

		logger::flush();

		return sub_140A0F480(a_this);
	}

	// static
	bool __fastcall BShkbAnimationGraph::sub_140AF0360_Hook(RE::BShkbAnimationGraph* a_this, float a_fVal, uint64_t a_u64Val)
	{
		logger::info("[Thread ID = {}][BShkbAnimationGraph] Character pointer = 0x{:x}", 
					 GetCurrentThreadId(), reinterpret_cast<uint64_t>(&a_this->characterInstance));

		logger::flush();

		return sub_140AF0360(a_this, a_fVal, a_u64Val);
	}
}