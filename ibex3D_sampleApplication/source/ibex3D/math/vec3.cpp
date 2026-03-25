#include <ibex3D/math/vec3.h>

#include <math.h>

// ----------------------------------------------------------------------------------------------------

vec3 vec3::operator + (const vec3& other) const
{
	// TODO: Implement SIMD later!
	return vec3(x + other.x, y + other.y, z + other.z);
}

vec3 vec3::operator - (const vec3& other) const
{
	return vec3(x - other.x, y - other.y, z - other.z);
}

vec3 vec3::operator * (const vec3& other) const
{
	return vec3(x * other.x, y - other.y, z - other.z);
}

vec3 vec3::operator / (const vec3& other) const
{	
	if (other.x == 0.0f) return vec3(0.0f);
	if (other.y == 0.0f) return vec3(0.0f);
	if (other.z == 0.0f) return vec3(0.0f);
	
	return vec3(x / other.x, y / other.y, z / other.z);
}

void vec3::operator += (const vec3& other)
{
	x += other.x;
	y += other.y;
	z += other.z;
}

void vec3::operator -= (const vec3& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
}

void vec3::operator *= (const vec3& other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
}

void vec3::operator /= (const vec3& other)
{
	if (other.x != 0.0f)
	{
		if (other.y != 0.0f)
		{
			if (other.z != 0.0f)
			{
				x /= other.x;
				y /= other.y;
				z /= other.z;
				return;
			}
		}
	}
	
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

bool vec3::operator == (const vec3& other) const
{
	return (x == other.x) && (y == other.y) && (z == other.z);
}

bool vec3::operator != (const vec3& other) const
{
	return (x != other.x) || (y != other.y) || (z != other.z);
}

// ----------------------------------------------------------------------------------------------------

vec3 vec3::dividedByFloat(float in) const
{
	if (in == 0.0f) return vec3(0.0f);
	
	return vec3(x / in, y / in, z / in);
}

void vec3::divideByFloat(float in)
{
	if (in == 0.0f)
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	else
	{
		x /= in;
		y /= in;
		z /= in;
	}
}

float vec3::getLength() const
{
	return sqrtf((x*x) + (y*y) + (z*z));
}

float vec3::getDotProduct(const vec3& other) const
{
	return (x * other.x) + (y * other.y) + (z * other.z);
}

vec3 vec3::normalized() const
{
	float len = getLength();

	if (len == 0.0f)
	{
		return vec3(0.0f);
	}

	return vec3(x / len, y / len, z / len);
}

void vec3::normalize()
{
	float len = getLength();

	if (len != 0.0f)
	{
		x /= len;
		y /= len;
		z /= len;
	}

	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

// ----------------------------------------------------------------------------------------------------

vec3 vec3::unsafeDividedBy(const vec3& other) const
{
	return vec3(x / other.x, y / other.y, z / other.z);
}

vec3 vec3::unsafeDividedByFloat(float in) const
{
	return vec3(x / in, y / in, z / in);
}

void vec3::unsafeDivideBy(const vec3& other)
{
	x /= other.x;
	y /= other.y;
	z /= other.z;
}

void vec3::unsafeDivideByFloat(float in)
{
	x /= in;
	y /= in;
	z /= in;
}

vec3 vec3::unsafeNormalized() const
{
	float len = getLength();
	return vec3(x / len, y / len, z / len);
}

void vec3::unsafeNormalize()
{
	float len = getLength();
	x /= len;
	y /= len;
	z /= len;
}

// ----------------------------------------------------------------------------------------------------

size_t vec3::getSize()
{
	size_t size = sizeof(float);
	return size + (size << 1);
}