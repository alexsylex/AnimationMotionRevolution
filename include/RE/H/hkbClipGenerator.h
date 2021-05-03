#pragma once

#include "RE/H/hkbGenerator.h"
#include "RE/H/hkbContext.h"
#include "RE/H/hkaDefaultAnimationControl.h"

namespace RE
{
	class hkbClipTriggerArray;

	class hkbClipGenerator : public hkbGenerator
	{
	public:
		inline static constexpr auto RTTI = RTTI_hkbClipGenerator;

		virtual ~hkbClipGenerator(); 	// 00

		// override (hkbNode)
		virtual void Activate(const hkbContext& a_context) override;	// 04

		// members
		hkStringPtr 							animationName;		// 48
		hkRefPtr<hkbClipTriggerArray> 			triggers;			// 50
		std::uint64_t 							unk58[6];			// 58
		hkRefPtr<hkaDefaultAnimationControl>	animationControl;	// 88
		hkRefPtr<hkbClipTriggerArray>			originalTriggers;	// 90
		std::uint64_t							unk98;				// 98
		hkaAnimationBinding* 					binding;			// A0
		// ...
	};
	static_assert(sizeof(hkbClipGenerator) >= 0xA8);
}