#include <ibex3D/math/vec3.h>
#include <math.h>

// ----------------------------------------------------------------------------------------------------

i3D_vec3 i3D_vec3::operator + (const i3D_vec3& other) const
{
	// TODO: Implement SIMD soon!
	return i3D_vec3(x + other.x, y + other.y, z + other.z);
}

i3D_vec3 i3D_vec3::operator - (const i3D_vec3& other) const
{
	return i3D_vec3(x - other.x, y - other.y, z - other.z);
}

i3D_vec3 i3D_vec3::operator * (const i3D_vec3& other) const
{
	return i3D_vec3(x * other.x, y * other.y, z * other.z);
}

i3D_vec3 i3D_vec3::operator / (const i3D_vec3& other) const
{	
	if (other.x == 0.0f) return i3D_vec3(0.0f);
	if (other.y == 0.0f) return i3D_vec3(0.0f);
	if (other.z == 0.0f) return i3D_vec3(0.0f);
	
	return i3D_vec3(x / other.x, y / other.y, z / other.z);
}

void i3D_vec3::operator += (const i3D_vec3& other)
{
	x += other.x;
	y += other.y;
	z += other.z;
}

void i3D_vec3::operator -= (const i3D_vec3& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
}

void i3D_vec3::operator *= (const i3D_vec3& other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
}

void i3D_vec3::operator /= (const i3D_vec3& other)
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

bool i3D_vec3::operator == (const i3D_vec3& other) const
{
	if (x != other.x) return false;
	if (y != other.y) return false;
	if (z != other.z) return false;
	return true;
}

bool i3D_vec3::operator != (const i3D_vec3& other) const
{
	if (x != other.x) return true;
	if (y != other.y) return true;
	if (z != other.z) return true;
	return false;
}

// ----------------------------------------------------------------------------------------------------

i3D_vec3 i3D_vec3::multipliedByFloat(float in) const
{
	return i3D_vec3(x * in, y * in, z * in);
}

void i3D_vec3::multiplyByFloat(float in)
{
	x *= in;
	y *= in;
	z *= in;
}

i3D_vec3 i3D_vec3::dividedByFloat(float in) const
{
	if (in == 0.0f) return i3D_vec3(0.0f);
	
	return i3D_vec3(x / in, y / in, z / in);
}

void i3D_vec3::divideByFloat(float in)
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

i3D_vec3 i3D_vec3::lerpedTo(const i3D_vec3& other, float amount) const
{
	return i3D_vec3
	(
		x + ((other.x - x) * amount),
		y + ((other.y - y) * amount),
		z + ((other.z - z) * amount)
	);
}

void i3D_vec3::lerpTo(const i3D_vec3& other, float amount)
{
	x += (other.x - x) * amount;
	y += (other.y - y) * amount;
	z += (other.z - z) * amount;
}

float i3D_vec3::getLength() const
{
	return sqrtf((x*x) + (y*y) + (z*z));
}

i3D_vec3 i3D_vec3::normalized() const
{
	float len = getLength();

	if (len == 0.0f)
	{
		return i3D_vec3(0.0f);
	}

	return i3D_vec3(x / len, y / len, z / len);
}

void i3D_vec3::normalize()
{
	float len = getLength();

	if (len == 0.0f)
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	else
	{
		x /= len;
		y /= len;
		z /= len;
	}
}

float i3D_vec3::getDotProduct(const i3D_vec3& other) const
{
	return (x * other.x) + (y * other.y) + (z * other.z);
}

// ----------------------------------------------------------------------------------------------------

i3D_vec3 i3D_vec3::unsafeDividedBy(const i3D_vec3& other) const
{
	return i3D_vec3(x / other.x, y / other.y, z / other.z);
}

i3D_vec3 i3D_vec3::unsafeDividedByFloat(float in) const
{
	return i3D_vec3(x / in, y / in, z / in);
}

void i3D_vec3::unsafeDivideBy(const i3D_vec3& other)
{
	x /= other.x;
	y /= other.y;
	z /= other.z;
}

void i3D_vec3::unsafeDivideByFloat(float in)
{
	x /= in;
	y /= in;
	z /= in;
}

i3D_vec3 i3D_vec3::unsafeNormalized() const
{
	float len = getLength();
	return i3D_vec3(x / len, y / len, z / len);
}

void i3D_vec3::unsafeNormalize()
{
	float len = getLength();
	x /= len;
	y /= len;
	z /= len;
}
