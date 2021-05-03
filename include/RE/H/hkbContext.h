#pragma once

#include "RE/H/hkbCharacter.h"

namespace RE
{
	class hkbContext
	{
	public:

		// members
		hkbCharacter* character; // 00
		// ...
	};
	static_assert(sizeof(hkbContext) >= 0x08);
}