#pragma once

#include <cstdint>

#include <immintrin.h>

class half
{
public:

	half(std::uint16_t a_value)
	: value{ a_value }
	{ }

	half(float a_value)
	: value{ _mm_cvtps_ph([&a_value]() -> __m128 
						  {
							__m128 value128;
							value128.m128_f32[0] = a_value;

							return value128;
						  }(), 1).m128i_u16[0] }
	{ }

	inline operator float()
	{
		__m128i value128;

		value128.m128i_u16[0] = value;

		return _mm_cvtph_ps(value128).m128_f32[0];
	}

private:

	std::uint16_t value;
};