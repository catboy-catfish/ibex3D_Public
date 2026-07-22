#pragma once

// ----------------------------------------------------------------------------------------------------

struct i3D_vec4
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;

	i3D_vec4() = default;
	i3D_vec4(float inX, float inY, float inZ, float inW) : x(inX), y(inY), z(inZ), w(inW) {}
	i3D_vec4(float inXYZW) : x(inXYZW), y(inXYZW), z(inXYZW), w(inXYZW) {}

	// ----------------------------------------------------------------------------------------------------

	i3D_vec4 operator + (const i3D_vec4& other) const;
	i3D_vec4 operator - (const i3D_vec4& other) const;
	i3D_vec4 operator * (const i3D_vec4& other) const;
	i3D_vec4 operator / (const i3D_vec4& other) const;

	void operator += (const i3D_vec4& other);
	void operator -= (const i3D_vec4& other);
	void operator *= (const i3D_vec4& other);
	void operator /= (const i3D_vec4& other);

	bool operator == (const i3D_vec4& other) const;
	bool operator != (const i3D_vec4& other) const;

	// ----------------------------------------------------------------------------------------------------

	i3D_vec4 multipliedByFloat(float in) const;
	void multiplyByFloat(float in);

	i3D_vec4 dividedByFloat(float in) const;
	void divideByFloat(float in);

	i3D_vec4 lerpedTo(const i3D_vec4& other, float amount) const;
	void lerpTo(const i3D_vec4& other, float amount);

	float getLength() const;

	i3D_vec4 normalized() const;
	void normalize();

	float getDotProduct(const i3D_vec4& other) const;

	// ----------------------------------------------------------------------------------------------------

	i3D_vec4 unsafeDividedBy(const i3D_vec4& other) const;
	i3D_vec4 unsafeDividedByFloat(float in) const;
	void unsafeDivideBy(const i3D_vec4& other);
	void unsafeDivideByFloat(float in);

	i3D_vec4 unsafeNormalized() const;
	void unsafeNormalize();
};