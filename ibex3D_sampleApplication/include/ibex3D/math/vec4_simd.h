#pragma once

#include <immintrin.h>

// ----------------------------------------------------------------------------------------------------

struct alignas(16) vec4_simd
{
	union
	{
		float f[4];
		__m128 v;
	};

	vec4_simd() : v(_mm_setzero_ps()) {}
	vec4_simd(float x, float y, float z, float w) : v(_mm_set_ps(w, z, y, x)) {}
	vec4_simd(__m128 in) : v(in) {};

	vec4_simd operator + (const vec4_simd& other) const
	{
		return vec4_simd(_mm_add_ps(v, other.v));
	}

	void operator += (const vec4_simd& other)
	{
		v = _mm_add_ps(v, other.v);
	}
};