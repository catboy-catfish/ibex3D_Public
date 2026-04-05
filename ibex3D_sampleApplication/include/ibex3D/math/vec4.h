#pragma once

struct vec4
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;

	vec4() = default;
	vec4(float inX, float inY, float inZ, float inW) : x(inX), y(inY), z(inZ), w(inW) {}
	vec4(float inXYZW) : x(inXYZW), y(inXYZW), z(inXYZW), w(inXYZW) {}

	// ----------------------------------------------------------------------------------------------------

	vec4 operator + (const vec4& other) const;
	vec4 operator - (const vec4& other) const;
	vec4 operator * (const vec4& other) const;
	vec4 operator / (const vec4& other) const;

	void operator += (const vec4& other);
	void operator -= (const vec4& other);
	void operator *= (const vec4& other);
	void operator /= (const vec4& other);

	bool operator == (const vec4& other) const;
	bool operator != (const vec4& other) const;

	// ----------------------------------------------------------------------------------------------------

	vec4 multipliedByFloat(float in) const;
	void multiplyByFloat(float in);

	vec4 dividedByFloat(float in) const;
	void divideByFloat(float in);

	vec4 lerpedTo(const vec4& other, float amount) const;
	void lerpTo(const vec4& other, float amount);

	float getLength() const;

	vec4 normalized() const;
	void normalize();

	float getDotProduct(const vec4& other) const;

	// ----------------------------------------------------------------------------------------------------

	vec4 unsafeDividedBy(const vec4& other) const;
	vec4 unsafeDividedByFloat(float in) const;
	void unsafeDivideBy(const vec4& other);
	void unsafeDivideByFloat(float in);

	vec4 unsafeNormalized() const;
	void unsafeNormalize();

	// ----------------------------------------------------------------------------------------------------

	static size_t getSize();
};