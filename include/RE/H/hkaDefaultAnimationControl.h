#pragma once

#include "RE/H/hkaAnimationControl.h"
#include "RE/H/hkVector4.h"

namespace RE
{
	class hkaDefaultAnimationControlMapperData;
	class hkaDefaultAnimationControlListener;

	class hkaDefaultAnimationControl : public hkaAnimationControl
	{
	public:

		enum class EaseStatus
		{
			EASING_IN = 0,
			EASED_IN,
			EASING_OUT,
			EASED_OUT
		};

		float										masterWeight;				// 58
		float										playbackSpeed;				// 5C
		std::uint32_t								overflowCount;				// 60
		std::uint32_t								underflowCount;				// 64
		std::int32_t								maxCycles;					// 68
		std::uint32_t								pad6C;						// 6C
		hkVector4									easeInCurve;				// 70
		hkVector4									easeOutCurve;				// 80
		float										easeInvDuration;			// 90
		float										easeT;						// 94
		EaseStatus									easeStatus;					// 98
		float										cropStartAmountLocalTime;	// 9C
		float										cropEndAmountLocalTime;		// A0
		std::uint32_t 								padA4;						// A4
		hkArray<hkaDefaultAnimationControlListener>	defaultListeners;			// A8
		hkaDefaultAnimationControlMapperData* 		mapper;						// B8
	};
}