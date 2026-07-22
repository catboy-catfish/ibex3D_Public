#pragma once

// ----------------------------------------------------------------------------------------------------

struct i3D_vec2
{
	float x = 0.0f;
	float y = 0.0f;

	i3D_vec2() = default;
	i3D_vec2(float inX, float inY) : x(inX), y(inY) {}
	i3D_vec2(float inXY) : x(inXY), y(inXY) {}

	// ----------------------------------------------------------------------------------------------------

	i3D_vec2 operator + (const i3D_vec2& other) const;
	i3D_vec2 operator - (const i3D_vec2& other) const;
	i3D_vec2 operator * (const i3D_vec2& other) const;
	i3D_vec2 operator / (const i3D_vec2& other) const;
	void operator += (const i3D_vec2& other);
	void operator -= (const i3D_vec2& other);
	void operator *= (const i3D_vec2& other);
	void operator /= (const i3D_vec2& other);
	
	bool operator == (const i3D_vec2& other) const;
	bool operator != (const i3D_vec2& other) const;

	// ----------------------------------------------------------------------------------------------------

	i3D_vec2 multipliedByFloat(float in) const;
	void multiplyByFloat(float in);
	i3D_vec2 dividedByFloat(float in) const;
	void divideByFloat(float in);

	i3D_vec2 lerpedTo(const i3D_vec2& other, float amount) const;
	void lerpTo(const i3D_vec2& other, float amount);

	float getLength() const;
	i3D_vec2 normalized() const;
	void normalize();

	float getDotProduct(const i3D_vec2& other) const;

	// ----------------------------------------------------------------------------------------------------

	i3D_vec2 unsafeDividedBy(const i3D_vec2& other) const;
	i3D_vec2 unsafeDividedByFloat(float in) const;
	void unsafeDivideBy(const i3D_vec2& other);
	void unsafeDivideByFloat(float in);

	i3D_vec2 unsafeNormalized() const;
	void unsafeNormalize();
};