#pragma once

#include "RE/B/bhkCharacterState.h"

namespace RE
{
	class bhkCharacterStateClimbing : public bhkCharacterState
	{
	public:
		inline static constexpr auto RTTI = RTTI_bhkCharacterStateClimbing;

		~bhkCharacterStateClimbing() override;	// 00

		// override (bhkCharacterState)
		hkpCharacterStateType GetType() const override;							   // 03 - { return kClimbing; }
		void SimulateStatePhysics(bhkCharacterController* a_controller) override;  // 08
	};
	static_assert(sizeof(bhkCharacterStateClimbing) == 0x10);
}
