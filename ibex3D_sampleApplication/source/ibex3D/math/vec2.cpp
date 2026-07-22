#include <ibex3D/math/vec2.h>
#include <math.h>

// ----------------------------------------------------------------------------------------------------

i3D_vec2 i3D_vec2::operator + (const i3D_vec2& other) const
{
	// TODO: Implement SIMD soon!
	return i3D_vec2(x + other.x, y + other.y);
}

i3D_vec2 i3D_vec2::operator - (const i3D_vec2& other) const
{
	return i3D_vec2(x - other.x, y - other.y);
}

i3D_vec2 i3D_vec2::operator * (const i3D_vec2& other) const
{
	return i3D_vec2(x * other.x, y * other.y);
}

i3D_vec2 i3D_vec2::operator / (const i3D_vec2& other) const
{
	if (other.x == 0.0f) return i3D_vec2(0.0f);
	if (other.y == 0.0f) return i3D_vec2(0.0f);

	return i3D_vec2(x / other.x, y / other.y);
}

void i3D_vec2::operator += (const i3D_vec2& other)
{
	x += other.x;
	y += other.y;
}

void i3D_vec2::operator -= (const i3D_vec2& other)
{
	x -= other.x;
	y -= other.y;
}

void i3D_vec2::operator *= (const i3D_vec2& other)
{
	x *= other.x;
	y *= other.y;
}

void i3D_vec2::operator /= (const i3D_vec2& other)
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

bool i3D_vec2::operator == (const i3D_vec2& other) const
{
	if (x != other.x) return false;
	if (y != other.y) return false;
	return true;
}

bool i3D_vec2::operator != (const i3D_vec2& other) const
{
	if (x != other.x) return true;
	if (y != other.y) return true;
	return false;
}

// ----------------------------------------------------------------------------------------------------

i3D_vec2 i3D_vec2::multipliedByFloat(float in) const
{
	return i3D_vec2(x * in, y * in);
}

void i3D_vec2::multiplyByFloat(float in)
{
	x *= in;
	y *= in;
}

i3D_vec2 i3D_vec2::dividedByFloat(float in) const
{
	if (in == 0.0f) return i3D_vec2(0.0f);

	return i3D_vec2(x / in, y / in);
}

void i3D_vec2::divideByFloat(float in)
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

i3D_vec2 i3D_vec2::lerpedTo(const i3D_vec2& other, float amount) const
{
	return i3D_vec2
	(
		x + ((other.x - x) * amount),
		y + ((other.y - y) * amount)
	);
}

void i3D_vec2::lerpTo(const i3D_vec2& other, float amount)
{
	x += (other.x - x) * amount;
	y += (other.y - y) * amount;
}

float i3D_vec2::getLength() const
{
	return sqrtf((x * x) + (y * y));
}

i3D_vec2 i3D_vec2::normalized() const
{
	float len = getLength();

	if (len == 0.0f)
	{
		return i3D_vec2(0.0f);
	}

	return i3D_vec2(x / len, y / len);
}

void i3D_vec2::normalize()
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

float i3D_vec2::getDotProduct(const i3D_vec2& other) const
{
	return (x * other.x) + (y * other.y);
}

// ----------------------------------------------------------------------------------------------------

i3D_vec2 i3D_vec2::unsafeDividedBy(const i3D_vec2& other) const
{
	return i3D_vec2(x / other.x, y / other.y);
}

i3D_vec2 i3D_vec2::unsafeDividedByFloat(float in) const
{
	return i3D_vec2(x / in, y / in);
}

void i3D_vec2::unsafeDivideBy(const i3D_vec2& other)
{
	x /= other.x;
	y /= other.y;
}

void i3D_vec2::unsafeDivideByFloat(float in)
{
	x /= in;
	y /= in;
}

i3D_vec2 i3D_vec2::unsafeNormalized() const
{
	float len = getLength();
	return i3D_vec2(x / len, y / len);
}

void i3D_vec2::unsafeNormalize()
{
	float len = getLength();

	x /= len;
	y /= len;
}
