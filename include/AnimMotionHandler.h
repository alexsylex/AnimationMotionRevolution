#pragma once

#include "Motion.h"

class AnimMotionData
{
public:

	AnimMotionData() = default;

	AnimMotionData(const RE::hkaAnimation* a_animation, const Translation* a_translation) :
		animation{ a_animation }, translationList{ *a_translation }
	{ }

	AnimMotionData(const RE::hkaAnimation* a_animation, const Rotation* a_rotation) :
		animation{ a_animation }, rotationList{ *a_rotation }
	{ }

	void Add(const Translation* a_translation)
	{
		translationList.push_back(*a_translation);
	}

	void Add(const Rotation* a_rotation)
	{
		rotationList.push_back(*a_rotation);
	}

	void SortListsByTime()
	{
		if (!translationList.empty())
		{
			std::sort(translationList.begin(), translationList.end(),
				[](const Translation& a_lhs, const Translation& a_rhs) -> bool
				{
					return a_lhs.time < a_rhs.time;
				});
		}

		if (!rotationList.empty())
		{
			std::sort(rotationList.begin(), rotationList.end(),
				[](const Rotation& a_lhs, const Rotation& a_rhs) -> bool
				{
					return a_lhs.time < a_rhs.time;
				});
		}
	}

	const RE::hkaAnimation* animation = nullptr;
	std::vector<Translation> translationList;
	std::vector<Rotation> rotationList;
	std::int32_t activeCount = 1;
};