#pragma once

#include "RE/B/bhkCharacterController.h"
#include "RE/H/hkpCharacterState.h"

namespace RE
{
	class bhkCharacterState : public hkpCharacterState
	{
	public:
		inline static constexpr auto RTTI = RTTI_bhkCharacterState;

		~bhkCharacterState() override;	// 00

		// override (hkpCharacterState)
		void Update(hkpCharacterContext& a_context, const hkpCharacterInput& a_input, hkpCharacterOutput& a_output) override;  // 06
		void Change(hkpCharacterContext& a_context, const hkpCharacterInput& a_input, hkpCharacterOutput& a_output) override;  // 07

		// add
		virtual void SimulateStatePhysics(bhkCharacterController* a_controller) = 0;	// 08
	};
	static_assert(sizeof(bhkCharacterState) == 0x10);
}
