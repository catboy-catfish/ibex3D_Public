#pragma once

struct vec2
{
	float x = 0.0f;
	float y = 0.0f;

	vec2() = default;
	vec2(float inX, float inY) : x(inX), y(inY) {}
	vec2(float inXY) : x(inXY), y(inXY) {}

	// ----------------------------------------------------------------------------------------------------

	vec2 operator + (const vec2& other) const;
	vec2 operator - (const vec2& other) const;
	vec2 operator * (const vec2& other) const;
	vec2 operator / (const vec2& other) const;

	void operator += (const vec2& other);
	void operator -= (const vec2& other);
	void operator *= (const vec2& other);
	void operator /= (const vec2& other);

	bool operator == (const vec2& other) const;
	bool operator != (const vec2& other) const;

	// ----------------------------------------------------------------------------------------------------

	vec2 multipliedByFloat(float in) const;
	void multiplyByFloat(float in);

	vec2 dividedByFloat(float in) const;
	void divideByFloat(float in);

	float getLength() const;

	vec2 normalized() const;
	void normalize();

	float getDotProduct(const vec2& other) const;

	// ----------------------------------------------------------------------------------------------------

	vec2 unsafeDividedBy(const vec2& other) const;
	vec2 unsafeDividedByFloat(float in) const;
	void unsafeDivideBy(const vec2& other);
	void unsafeDivideByFloat(float in);

	vec2 unsafeNormalized() const;
	void unsafeNormalize();

	// ----------------------------------------------------------------------------------------------------

	static size_t getSize();
};
