#include <ibex3D/math/vec4.h>
#include <math.h>

#if defined(IBEX3D_SIMD_SSE)
#include <immintrin.h>
#endif

// ----------------------------------------------------------------------------------------------------

i3D_vec4 i3D_vec4::operator + (const i3D_vec4& other) const
{	
	return i3D_vec4(x + other.x, y + other.y, z + other.z, w + other.w);
}

i3D_vec4 i3D_vec4::operator - (const i3D_vec4& other) const
{
	return i3D_vec4(x - other.x, y - other.y, z - other.z, w - other.w);
}

i3D_vec4 i3D_vec4::operator * (const i3D_vec4& other) const
{
	return i3D_vec4(x * other.x, y * other.y, z * other.z, w * other.w);
}

i3D_vec4 i3D_vec4::operator / (const i3D_vec4& other) const
{
	if (other.x == 0.0f) return i3D_vec4(0.0f);
	if (other.y == 0.0f) return i3D_vec4(0.0f);
	if (other.z == 0.0f) return i3D_vec4(0.0f);
	if (other.w == 0.0f) return i3D_vec4(0.0f);
	
	return i3D_vec4(x / other.x, y / other.y, z / other.z, w / other.w);
}

void i3D_vec4::operator += (const i3D_vec4& other)
{
#ifdef IBEX3D_SIMD_SSE
	__m128 lhs = _mm_load_ps(&x);
	__m128 rhs = _mm_load_ps(&other.x);
	__m128 result = _mm_add_ps(lhs, rhs);
	_mm_store_ps(&x, result);
#else
	x += other.x;
	y += other.y;
	z += other.z;
	w += other.w;
#endif
}

void i3D_vec4::operator -= (const i3D_vec4& other)
{
#ifdef IBEX3D_SIMD_SSE
	__m128 lhs = _mm_load_ps(&x);
	__m128 rhs = _mm_load_ps(&other.x);
	__m128 result = _mm_sub_ps(lhs, rhs);
	_mm_store_ps(&x, result);
#else
	x -= other.x;
	y -= other.y;
	z -= other.z;
	w -= other.w;
#endif
}

void i3D_vec4::operator *= (const i3D_vec4& other)
{
#ifdef IBEX3D_SIMD_SSE
	__m128 lhs = _mm_load_ps(&x);
	__m128 rhs = _mm_load_ps(&other.x);
	__m128 result = _mm_mul_ps(lhs, rhs);
	_mm_store_ps(&x, result);
#else
	x *= other.x;
	y *= other.y;
	z *= other.z;
	w *= other.w;
#endif
}

void i3D_vec4::operator /= (const i3D_vec4& other)
{
	if ((other.x != 0.0f) && (other.y != 0.0f))
	{
		if ((other.z != 0.0f) && (other.w != 0.0f))
		{
#ifdef IBEX3D_SIMD_SSE
			__m128 lhs = _mm_load_ps(&x);
			__m128 rhs = _mm_load_ps(&other.x);
			__m128 result = _mm_div_ps(lhs, rhs);
			_mm_store_ps(&x, result);
#else
			x /= other.x;
			y /= other.y;
			z /= other.z;
			w /= other.w;
#endif
			return;
		}
	}

	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 0.0f;
}

bool i3D_vec4::operator == (const i3D_vec4& other) const
{
	if (x != other.x) return false;
	if (y != other.y) return false;
	if (z != other.z) return false;
	if (w != other.w) return false;
	return true;
}

bool i3D_vec4::operator != (const i3D_vec4& other) const
{
	if (x != other.x) return true;
	if (y != other.y) return true;
	if (z != other.z) return true;
	if (w != other.w) return true;
	return false;
}

// ----------------------------------------------------------------------------------------------------

i3D_vec4 i3D_vec4::multipliedByFloat(float in) const
{
	return i3D_vec4(x * in, y * in, z * in, w * in);
}

void i3D_vec4::multiplyByFloat(float in)
{
	x *= in;
	y *= in;
	z *= in;
	w *= in;
}

i3D_vec4 i3D_vec4::dividedByFloat(float in) const
{
	if (in == 0.0f) return i3D_vec4(0.0f);
	return i3D_vec4(x / in, y / in, z / in, w / in);
}

void i3D_vec4::divideByFloat(float in)
{
	if (in == 0.0f)
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 0.0f;
	}
	else
	{
		x /= in;
		y /= in;
		z /= in;
		w /= in;
	}
}

i3D_vec4 i3D_vec4::lerpedTo(const i3D_vec4& other, float amount) const
{
	return i3D_vec4
	(
		x + ((other.x - x) * amount),
		y + ((other.y - y) * amount),
		z + ((other.z - z) * amount),
		w + ((other.w - w) * amount)
	);
}

void i3D_vec4::lerpTo(const i3D_vec4& other, float amount)
{
	x += (other.x - x) * amount;
	y += (other.y - y) * amount;
	z += (other.z - z) * amount;
	w += (other.w - w) * amount;
}

float i3D_vec4::getLength() const
{
	return sqrtf((x*x) + (y*y) + (z*z) + (w*w));
}

i3D_vec4 i3D_vec4::normalized() const
{
	float len = getLength();

	if (len == 0.0f)
	{
		return i3D_vec4(0.0f);
	}
	
	return i3D_vec4(x / len, y / len, z / len, w / len);
}

void i3D_vec4::normalize()
{
	float len = getLength();

	if (len == 0.0f)
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 0.0f;
	}
	else
	{
		x /= len;
		y /= len;
		z /= len;
		w /= len;
	}
}

float i3D_vec4::getDotProduct(const i3D_vec4& other) const
{
	return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
}

// ----------------------------------------------------------------------------------------------------

i3D_vec4 i3D_vec4::unsafeDividedBy(const i3D_vec4& other) const
{
	return i3D_vec4(x / other.x, y / other.y, z / other.z, w / other.w);
}

i3D_vec4 i3D_vec4::unsafeDividedByFloat(float in) const
{
	return i3D_vec4(x / in, y / in, z / in, w / in);
}

void i3D_vec4::unsafeDivideBy(const i3D_vec4& other)
{
#ifdef IBEX3D_SIMD_SSE
	__m128 lhs = _mm_load_ps(&x);
	__m128 rhs = _mm_load_ps(&other.x);
	__m128 result = _mm_div_ps(lhs, rhs);
	_mm_store_ps(&x, result);
#else
	x /= other.x;
	y /= other.y;
	z /= other.z;
	w /= other.w;
#endif
}

void i3D_vec4::unsafeDivideByFloat(float in)
{
	x /= in;
	y /= in;
	z /= in;
	w /= in;
}

i3D_vec4 i3D_vec4::unsafeNormalized() const
{
	float len = getLength();
	return i3D_vec4(x / len, y / len, z / len, w / len);
}

void i3D_vec4::unsafeNormalize()
{
	float len = getLength();

	x /= len;
	y /= len;
	z /= len;
	w /= len;
}
