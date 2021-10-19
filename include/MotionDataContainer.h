#pragma once

#include "half.h"

namespace RE
{
	class MotionDataContainer
	{
	public:

		// Translation
		constexpr bool IsTranslationDataAligned() const
		{
			return !(translationDataPtr & 1);
		}

		inline NiPoint3& GetSegTranslation(int a_segIndex) const
		{
			return GetSegTranslationList()[a_segIndex - 1];
		}

		inline NiPoint3& GetEndTranslation() const
		{
			return GetSegTranslation(translationSegCount);
		}

		inline float GetSegTranslationTime(int a_segIndex) const
		{
			return GetSegTranslationTimesList()[a_segIndex - 1];
		}

		inline float GetEndTranslationTime() const
		{
			return GetSegTranslationTime(translationSegCount);
		}

		// Rotation
		constexpr bool IsRotationDataAligned() const
		{
			return !(rotationDataPtr & 1);
		}

		inline NiQuaternion& GetSegRotation(int a_segIndex) const
		{
			return GetSegRotationList()[a_segIndex - 1];
		}

		inline NiQuaternion& GetEndRotation() const
		{
			return GetSegRotation(rotationSegCount);
		}

		inline float GetSegRotationTime(int a_segIndex) const
		{
			return GetSegRotationTimesList()[a_segIndex - 1];
		}

		inline float GetEndRotationTime() const
		{
			return GetSegRotationTime(rotationSegCount);
		}

	private:

		// Translation
		inline std::uintptr_t TranslationDataAligned() const
		{
			return translationDataPtr & ~1;
		}

		inline NiPoint3* GetSegTranslationList() const
		{
			return reinterpret_cast<NiPoint3*>(TranslationDataAligned());
		}

		inline half* GetSegTranslationTimesList() const
		{
			return reinterpret_cast<half*>(&GetSegTranslationList()[translationSegCount]);
		}

		// Rotation
		inline std::uintptr_t RotationDataAligned() const
		{
			return rotationDataPtr & ~1;
		}

		inline NiQuaternion* GetSegRotationList() const
		{
			return reinterpret_cast<NiQuaternion*>(RotationDataAligned());
		}

		inline half* GetSegRotationTimesList() const
		{
			return reinterpret_cast<half*>(&GetSegRotationList()[rotationSegCount]);
		}

	public:

		// members
		std::uintptr_t	translationDataPtr;		// 00
		unsigned int	translationSegCount;	// 08
		std::uintptr_t	rotationDataPtr;		// 10
		unsigned int	rotationSegCount;		// 18
	};
}

static_assert(sizeof(std::array<char, 10>) == 10);