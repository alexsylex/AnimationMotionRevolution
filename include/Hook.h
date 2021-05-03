#pragma once

#include "RE/H/hkbClipGenerator.h"

#include "REL/Relocation.h"

namespace DMR
{
	class hkbClipGeneratorHook
	{
	public:

		static void Install()
		{
			SKSE::AllocTrampoline(1 << 4);

			SKSE::Trampoline& trampoline = SKSE::GetTrampoline();

			std::uint64_t hookOffset = 0x66E;	// E8 8D 32 00 00		call    hkbClipGenerator__sub_140A0F480
												// rcx = hkbClipGenerator*

			trampoline.write_call<5>(RE::Offset::hkbClipGenerator::Activate.address() + hookOffset, Hook);
		};

	private:

		static uint32_t __fastcall Hook(RE::hkbClipGenerator* a_this)
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
			
			logger::flush();

			using func_t = decltype(&Hook);
			REL::Relocation<func_t> hkbClipGenerator__sub_140A0F480{ REL::ID{ static_cast<uint64_t>(58628) } };
			return hkbClipGenerator__sub_140A0F480(a_this);
		}
	};

	class MotionDataContainerHook
	{
	public:

		static void Install()
		{
			SKSE::AllocTrampoline(1 << 4);

			SKSE::Trampoline& trampoline = SKSE::GetTrampoline();

			std::uint64_t hookOffset = 0; // ???

			trampoline.write_call<5>(RE::Offset::MotionDataContainer::Unk.address() + hookOffset, Hook);
		};

	private:

		static bool Hook(RE::hkbClipGenerator* a_this, const RE::hkbContext& a_context)
		{
			
		}
	};
}