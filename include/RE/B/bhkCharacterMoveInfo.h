#pragma once

#include "RE/N/NiPoint3.h"

namespace RE
{
	struct bhkCharacterMoveInfo
	{
		// members
		float secondsSinceLastFrame;  // 00
		NiPoint3 deltaRotation;		  // 04
		NiPoint3 deltaPosition;		  // 10
		float deltaSpeed;			  // 1C
		bool unk20;					  // 20
		std::uint8_t pad21;			  // 21
		std::uint16_t pad22;		  // 22
	};
	static_assert(sizeof(bhkCharacterMoveInfo) == 0x24);
}