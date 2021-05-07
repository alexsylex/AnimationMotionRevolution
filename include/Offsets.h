#pragma once

namespace RE
{
	namespace Offset
	{
		namespace hkbClipGenerator
		{
			constexpr REL::ID Activate(static_cast<std::uint64_t>(58602));

			// uint32_t hkbClipGenerator::sub_140A0F480(hkbClipGenerator* this)
			constexpr REL::ID sub_140A0F480(static_cast<uint64_t>(58628)); // hkbClipGenerator::Called from Activate

			constexpr REL::ID Vtbl(static_cast<std::uint64_t>(58632));
		}

		namespace BSAnimationGraphManager
		{
			constexpr REL::ID sub_140AE2E80(static_cast<std::uint64_t>(62431));
		}

		namespace BShkbAnimationGraph
		{
			// bool BShkbAnimationGraph::sub_140AF0360(BShkbAnimationGraph* this, float, uint64_t)
			constexpr REL::ID sub_140AF0360(static_cast<std::uint64_t>(62649)); // Called from BSAnimationGraphManager::sub_140AE2E80
		}

		namespace
		{
			// uint32_t __fastcall sub_1404DD5A0(uint64_t a_motionDataContainer, float a_fMotionTime, NiPoint3 *a_pos)
			constexpr REL::ID j_ApplyMotionData(static_cast<std::uint64_t>(31804));

			// uint64_t __fastcall sub_1404DDA20(uint64_t a_motionDataContainer, float a_fMotionTime, NiPoint3 *a_pos)
			constexpr REL::ID ApplyMotionData(static_cast<std::uint64_t>(31812)); // Called from j_ApplyMotionData
		}
	}
}