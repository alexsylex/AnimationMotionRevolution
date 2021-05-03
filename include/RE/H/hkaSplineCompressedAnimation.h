#pragma once

#include "RE/H/hkaAnimation.h"

namespace RE
{
	class hkaSplineCompressedAnimation : public hkaAnimation
	{
		std::int32_t			numFrames;					// 38
		std::int32_t			numBlocks;					// 3C
		std::int32_t			maxFramesPerBlock;			// 40
		std::int32_t			maskAndQuantizationSize;	// 44
		float					blockDuration;				// 48
		float					blockInverseDuration;		// 4C
		float					frameDuration;				// 50
		std::uint32_t			pad54;						// 54
		hkArray<std::uint32_t>	blockOffsets;				// 58
		hkArray<std::uint32_t>	floatBlockOffsets;			// 68
		hkArray<std::uint32_t>	transformOffsets;			// 78
		hkArray<std::uint32_t>	floatOffsets;				// 88
		hkArray<std::uint8_t>	data;						// 98
		std::int32_t			endian;						// A8
		std::uint32_t			padAC;						// AC
	};
	static_assert(sizeof(hkaSplineCompressedAnimation) == 0xB0);
}