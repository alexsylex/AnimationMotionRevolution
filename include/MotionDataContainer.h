#pragma once

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

		inline uint16_t& GetSegTranslationTime(int a_segIndex) const
		{
			return GetSegTranslationTimesList()[a_segIndex - 1];
		}

		inline uint16_t& GetEndTranslationTime() const
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

		inline uint16_t& GetSegRotationTime(int a_segIndex) const
		{
			return GetSegRotationTimesList()[a_segIndex - 1];
		}

		inline uint16_t& GetEndRotationTime() const
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

		inline uint16_t* GetSegTranslationTimesList() const
		{
			return reinterpret_cast<uint16_t*>(&GetSegTranslationList()[translationSegCount]);
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

		inline uint16_t* GetSegRotationTimesList() const
		{
			return reinterpret_cast<uint16_t*>(&GetSegRotationList()[rotationSegCount]);
		}

	public:

		// members
		std::uintptr_t	translationDataPtr;		// 00
		unsigned int	translationSegCount;	// 08
		std::uintptr_t	rotationDataPtr;		// 10
		unsigned int	rotationSegCount;		// 18
	};
}