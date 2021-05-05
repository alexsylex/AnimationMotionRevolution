#pragma once

namespace RE
{
	namespace Offset
	{
		namespace hkbClipGenerator
		{
			constexpr REL::ID Activate(static_cast<std::uint64_t>(58602));
			constexpr REL::ID Vtbl(static_cast<std::uint64_t>(58632));
		}

		namespace
		{
			// int64 __fastcall sub_1404DDA20(int64 a_motionDataContainer, float a_fMotionTime, NiPoint3 *a_pos)
			constexpr REL::ID ApplyMotionData(static_cast<std::uint64_t>(31812));

			// int32 __fastcall sub_1404DD5A0(int64 a_motionDataContainer, float a_fMotionTime, NiPoint3 *a_pos)
			constexpr REL::ID j_ApplyMotionData(static_cast<std::uint64_t>(31804));
		}
	}
}