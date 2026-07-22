#pragma once

#include <immintrin.h>

// ----------------------------------------------------------------------------------------------------

struct alignas(16) i3D_vec4_simd
{
	union
	{
		float f[4];
		__m128 v;
	};

	i3D_vec4_simd() : v(_mm_setzero_ps()) {}
	i3D_vec4_simd(float x, float y, float z, float w) : v(_mm_set_ps(w, z, y, x)) {}
	i3D_vec4_simd(__m128 in) : v(in) {};

	i3D_vec4_simd operator + (const i3D_vec4_simd& other) const
	{
		return i3D_vec4_simd(_mm_add_ps(v, other.v));
	}

	void operator += (const i3D_vec4_simd& other)
	{
		v = _mm_add_ps(v, other.v);
	}
};