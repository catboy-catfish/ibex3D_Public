#include <ibex3D/math/vec2.h>

#include <math.h>

// ----------------------------------------------------------------------------------------------------

vec2 vec2::operator + (const vec2& other) const
{
	// TODO: Implement SIMD soon!
	return vec2(x + other.x, y + other.y);
}

vec2 vec2::operator - (const vec2& other) const
{
	return vec2(x - other.x, y - other.y);
}

vec2 vec2::operator * (const vec2& other) const
{
	return vec2(x * other.x, y * other.y);
}

vec2 vec2::operator / (const vec2& other) const
{
	if (other.x == 0.0f) return vec2(0.0f);
	if (other.y == 0.0f) return vec2(0.0f);

	return vec2(x / other.x, y / other.y);
}

void vec2::operator += (const vec2& other)
{
	x += other.x;
	y += other.y;
}

void vec2::operator -= (const vec2& other)
{
	x -= other.x;
	y -= other.y;
}

void vec2::operator *= (const vec2& other)
{
	x *= other.x;
	y *= other.y;
}

void vec2::operator /= (const vec2& other)
{
	if (other.x != 0.0f)
	{
		if (other.y != 0.0f)
		{
			x /= other.x;
			y /= other.y;
			return;
		}
	}

	x = 0.0f;
	y = 0.0f;
}

bool vec2::operator == (const vec2& other) const
{
	if (x != other.x) return false;
	if (y != other.y) return false;
	return true;
}

bool vec2::operator != (const vec2& other) const
{
	if (x != other.x) return true;
	if (y != other.y) return true;
	return false;
}

// ----------------------------------------------------------------------------------------------------

vec2 vec2::multipliedByFloat(float in) const
{
	return vec2(x * in, y * in);
}

void vec2::multiplyByFloat(float in)
{
	x *= in;
	y *= in;
}

vec2 vec2::dividedByFloat(float in) const
{
	if (in == 0.0f) return vec2(0.0f);

	return vec2(x / in, y / in);
}

void vec2::divideByFloat(float in)
{
	if (in == 0.0f)
	{
		x = 0.0f;
		y = 0.0f;
	}
	else
	{
		x /= in;
		y /= in;
	}
}

vec2 vec2::lerpedTo(const vec2& other, float amount) const
{
	return vec2
	(
		x + ((other.x - x) * amount),
		y + ((other.y - y) * amount)
	);
}

void vec2::lerpTo(const vec2& other, float amount)
{
	x += (other.x - x) * amount;
	y += (other.y - y) * amount;
}

float vec2::getLength() const
{
	return sqrtf((x * x) + (y * y));
}

vec2 vec2::normalized() const
{
	float len = getLength();

	if (len == 0.0f)
	{
		return vec2(0.0f);
	}

	return vec2(x / len, y / len);
}

void vec2::normalize()
{
	float len = getLength();

	if (len == 0.0f)
	{
		x = 0.0f;
		y = 0.0f;
	}
	else
	{
		x /= len;
		y /= len;
	}
}

float vec2::getDotProduct(const vec2& other) const
{
	return (x * other.x) + (y * other.y);
}

// ----------------------------------------------------------------------------------------------------

vec2 vec2::unsafeDividedBy(const vec2& other) const
{
	return vec2(x / other.x, y / other.y);
}

vec2 vec2::unsafeDividedByFloat(float in) const
{
	return vec2(x / in, y / in);
}

void vec2::unsafeDivideBy(const vec2& other)
{
	x /= other.x;
	y /= other.y;
}

void vec2::unsafeDivideByFloat(float in)
{
	x /= in;
	y /= in;
}

vec2 vec2::unsafeNormalized() const
{
	float len = getLength();
	return vec2(x / len, y / len);
}

void vec2::unsafeNormalize()
{
	float len = getLength();

	x /= len;
	y /= len;
}

// ----------------------------------------------------------------------------------------------------

size_t vec2::getSize()
{
	return sizeof(float) << 1;
}