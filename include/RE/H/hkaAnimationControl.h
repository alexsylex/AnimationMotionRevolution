#pragma once

#include "RE/H/hkReferencedObject.h"
#include "RE/H/hkArray.h"
#include "RE/H/hkaAnimationBinding.h"

namespace RE
{
	class hkaAnimationControlListener;

	class hkaAnimationControl : public hkReferencedObject
	{
	public:

		float									localTime;				// 10
		float									weight;					// 14
		hkArray<std::uint8_t>					transformTrackWeights;	// 18
		hkArray<std::uint8_t>					floatTrackWeights;		// 28
		hkaAnimationBinding* 					binding;				// 38
		hkArray<hkaAnimationControlListener>	listeners;				// 40
		float									motionTrackWeight;		// 50
		std::uint32_t							pad54;					// 54
	};
	static_assert(sizeof(hkaAnimationControl) == 0x58);
}