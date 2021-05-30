#pragma once

namespace RE
{
	template<typename T>
	class MotionDataContainer
	{
	public:

		constexpr bool IsDataAligned() const
		{
			return !(dataPtr & 1);
		}

		inline T& GetSegMotionOffset(int a_segIndex) const
		{
			return GetSegMotionOffsetsList()[a_segIndex - 1];
		}

		inline T& GetEndMotionOffset() const
		{
			return GetSegMotionOffset(segCount);
		}

		inline uint16_t& GetSegMotionTime(int a_segIndex) const
		{
			return GetSegMotionTimesList()[a_segIndex - 1];
		}

		inline uint16_t& GetEndMotionTime() const
		{
			return GetSegMotionTime(segCount);
		}

	private:

		inline std::uintptr_t DataAligned() const
		{
			return dataPtr & ~1;
		}

		inline T* GetSegMotionOffsetsList() const
		{
			return reinterpret_cast<T*>(DataAligned());
		}

		inline uint16_t* GetSegMotionTimesList() const
		{
			return reinterpret_cast<uint16_t*>(&GetSegMotionOffsetsList()[segCount]);
		}

		// members
		std::uintptr_t dataPtr;	// 00

	public:

		unsigned int segCount;	// 08
	};

	using MotionPositionContainer = MotionDataContainer<RE::NiPoint3>;
}