#pragma once

struct vec3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	vec3() = default;
	vec3(float inX, float inY, float inZ) : x(inX), y(inY), z(inZ) {}
	vec3(float inXYZ) : x(inXYZ), y(inXYZ), z(inXYZ) {}

	// ----------------------------------------------------------------------------------------------------

	vec3 operator + (const vec3& other) const;
	vec3 operator - (const vec3& other) const;
	vec3 operator * (const vec3& other) const;
	vec3 operator / (const vec3& other) const;

	void operator += (const vec3& other);
	void operator -= (const vec3& other);
	void operator *= (const vec3& other);
	void operator /= (const vec3& other);

	bool operator == (const vec3& other) const;
	bool operator != (const vec3& other) const;

	// ----------------------------------------------------------------------------------------------------

	vec3 multipliedByFloat(float in) const;
	void multiplyByFloat(float in);

	vec3 dividedByFloat(float in) const;
	void divideByFloat(float in);

	vec3 lerpedTo(const vec3& other, float amount) const;
	void lerpTo(const vec3& other, float amount);

	float getLength() const;

	vec3 normalized() const;
	void normalize();

	float getDotProduct(const vec3& other) const;

	// ----------------------------------------------------------------------------------------------------

	vec3 unsafeDividedBy(const vec3& other) const;
	vec3 unsafeDividedByFloat(float in) const;
	void unsafeDivideBy(const vec3& other);
	void unsafeDivideByFloat(float in);

	vec3 unsafeNormalized() const;
	void unsafeNormalize();

	// ----------------------------------------------------------------------------------------------------

	static size_t getSize();
};