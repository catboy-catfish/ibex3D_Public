#pragma once

// ----------------------------------------------------------------------------------------------------

struct i3D_vec3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	i3D_vec3() = default;
	i3D_vec3(float inX, float inY, float inZ) : x(inX), y(inY), z(inZ) {}
	i3D_vec3(float inXYZ) : x(inXYZ), y(inXYZ), z(inXYZ) {}

	// ----------------------------------------------------------------------------------------------------

	i3D_vec3 operator + (const i3D_vec3& other) const;
	i3D_vec3 operator - (const i3D_vec3& other) const;
	i3D_vec3 operator * (const i3D_vec3& other) const;
	i3D_vec3 operator / (const i3D_vec3& other) const;
	void operator += (const i3D_vec3& other);
	void operator -= (const i3D_vec3& other);
	void operator *= (const i3D_vec3& other);
	void operator /= (const i3D_vec3& other);

	bool operator == (const i3D_vec3& other) const;
	bool operator != (const i3D_vec3& other) const;

	// ----------------------------------------------------------------------------------------------------

	i3D_vec3 multipliedByFloat(float in) const;
	void multiplyByFloat(float in);
	i3D_vec3 dividedByFloat(float in) const;
	void divideByFloat(float in);

	i3D_vec3 lerpedTo(const i3D_vec3& other, float amount) const;
	void lerpTo(const i3D_vec3& other, float amount);

	float getLength() const;
	i3D_vec3 normalized() const;
	void normalize();

	float getDotProduct(const i3D_vec3& other) const;

	// ----------------------------------------------------------------------------------------------------

	i3D_vec3 unsafeDividedBy(const i3D_vec3& other) const;
	i3D_vec3 unsafeDividedByFloat(float in) const;
	void unsafeDivideBy(const i3D_vec3& other);
	void unsafeDivideByFloat(float in);

	i3D_vec3 unsafeNormalized() const;
	void unsafeNormalize();
};