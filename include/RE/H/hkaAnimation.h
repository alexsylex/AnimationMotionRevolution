#pragma once

#include "RE/H/hkReferencedObject.h"
#include "RE/H/hkRefPtr.h"
#include "RE/H/hkaAnnotationTrack.h"

namespace RE
{
	class hkaAnimatedReferenceFrame;

	class hkaAnimation : public hkReferencedObject
	{
	public:

		enum class AnimationType
		{
			HK_UNKNOWN_ANIMATION = 0,
			HK_INTERLEAVED_ANIMATION,
			HK_DELTA_COMPRESSED_ANIMATION,
			HK_WAVELET_COMPRESSED_ANIMATION,
			HK_MIRRORED_ANIMATION,
			HK_SPLINE_COMPRESSED_ANIMATION,
			HK_QUANTIZED_COMPRESSED_ANIMATION,
		};

		stl::enumeration<AnimationType, std::uint32_t>	type;						// 10
		float											duration;					// 14
		std::int32_t									numberOfTransformTracks;	// 18
		std::int32_t									numberOfFloatTracks;		// 1C
		hkRefPtr<hkaAnimatedReferenceFrame>				extractedMotion;			// 20
		hkArray<hkaAnnotationTrack>						annotationTracks;			// 28
	};
	static_assert(sizeof(hkaAnimation) == 0x38);
}