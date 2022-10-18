#pragma once

#include "RE/H/hkVector4.h"

namespace RE
{
	class hkpCharacterMovementUtil
	{
	public:
	
		struct hkpMovementUtilInput
		{
			/// Forward direction in world space
			hkVector4 forward;

			/// Up direction in world space
			hkVector4 up;

			/// Normal of the surface we're standing on in world space
			hkVector4 surfaceNormal;

			/// Our current velocity in world space
			hkVector4 currentVelocity;

			/// Our desired velocity in the surface frame
			hkVector4 desiredVelocity;

			/// Velocity of the surface we're standing on in world space
			hkVector4 surfaceVelocity;

			/// Gain for the character controller.
			/// This variable controls the acceleration of the character. It should be
			/// scaled by the current timestep to ensure that the characters acceleration
			/// is not timestep dependent.
			float gain;

			/// Limit the maximum acceleration of the character
			float maxVelocityDelta;
		};

		/// Calculate a new output velocity based on the input
		static void CalculateMovement(const hkpMovementUtilInput& a_input, hkVector4& a_velocityOut)
		{
			using func_t = decltype(CalculateMovement);
			REL::Relocation<func_t> func{ RELOCATION_ID(78988, 81002) };

			func(a_input, a_velocityOut);
		}
	};

}