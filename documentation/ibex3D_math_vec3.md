# vec3 - Basic overview

- Header file: `include/ibex3D/math/vec3.h`
- Source file: `source/ibex3D/math/vec3.cpp`

### Table of Contents

- [Description](#description)
- [Constructors](#constructors)
- [Operators](#operators)
  - [Arithmetic](#arithmetic)
  - [Arithmetic-assignment](#arithmetic-assignment)
  - [Logical](logical)
- [Functions](#functions)
  - [Scalar](#scalar)
  - [Advanced math](#advanced-math)
  - [Unsafe division](#unsafe-division)
- [Member variables](#member-variables)
- [Remarks](#remarks)
- [Examples](#examples)
- [To-do list](#to-do-list)

### Description

`vec3` is a simple struct which contains three single-precision floats bundled together, and also includes overrides/functions for various mathematical operations.

### Constructors

`vec3() = default`
- The default constructor (with no parameters). This leaves all coordinates of the vec3 with a value of zero.
- The reason why this entire declaration is in place is because when an overloaded constructor (with parameters) exists, the compiler assumes that that is the default constructor and refuses to create an extra parameter-less constructor by default, making it impossible to create a vec3 without specifying its values. Declaring `vec3() = default;` is effectively telling the compiler "Actually, no, _this_ is the default constructor" and makes it possible to create a vec3 again without having to specify its values.

`vec3(float inX, float inY, float inZ) : x(inX), y(inY), z(inZ) {}`
- An overloaded constructor which allows you to manually specify the X, Y and Z components of the vec3.
- The syntax `x(inX), y(inY), ...` automatically assigns the value of the parameter `inX` to the member variable `x`, assigns the value of `inY` to `y`, and so on. This is functionally equivalent to writing `{ x = inX; y = inY; ... }` in the constructor body (the brackets at the end).

`vec3(float inXYZ) : x(inXYZ), y(inXYZ), z(inXYZ) {}`
- An overloaded constructor which allows you to set every component of the vec3 to the single float parameter passed in. For example, if you were to say `vec3 myVec3(3.0f)`, `myVec3` would have the values `{ x=3.0f, y=3.0f, z=3.0f }`.

### Operators

##### Arithmetic

Addition (`vec3 operator + (const vec3& other) const`)
- Returns a new vec3 created from the components of this vec3 and `other` added together. This doesn't modify the values of this vec3 or `other`. 
- The argument `const vec3& other` is used instead of just `vec3 other` because the former passes `other` by reference, while the latter creates an unwanted copy of `other` when it's passed to the operator. In the case of `vec3`, it's very small, but I feel like it's still good practice in functions like this.
- The `const` qualifier at the start of the parameter `other` prevents the code in the function body from modifying it. `const` is used again at the end of the function declaration, and this time it means the code in the function body cannot modify its own variables. It's only natural to do this in this kind of function because it does neither of these.

Subtraction (`vec3 operator - (const vec3& other) const`)
- Returns a new vec3 created from the components of `other` subtracted from this vec3. This doesn't modify the values of this vec3 or `other`.

Multiplication (`vec3 operator * (const vec3& other) const`)
- Returns a new vec3 created from the components of this vec3 and `other` multiplied together. This doesn't modify the values of this vec3 or `other`.

Division (`vec3 operator / (const vec3& other) const`)
- Returns a new vec3 created from the components of this vec3 divided by `other`. This doesn't modify the values of this vec3 or `other`.
- This operator includes some safety checks which cause it to return `vec3(0.0f)` if any of `other`'s components are 0 to prevent division-by-zero errors. If you need a version of this operator without these checks, see `unsafeDividedBy()`.

##### Arithmetic assignment

Add-equal (`void operator += (const vec3& other)`)
- Adds the components of `other` to this vec3, which is functionally equivalent to the statement `myVec3 = myVec3 + myOtherVec3;`. This function doesn't create a new vec3, it directly modifies the current vec3, which is why it doesn't have any return value.
- The `const` qualifier at the end of the declaration has been removed because it now modifies its own variables.

Subtract-equal (`void operator -= (const vec3& other)`)
- Subtracts the components of `other` from this vec3, which is functionally equivalent to the statement `myVec3 = myVec3 - myOtherVec3;`.

Multiply-equal (`void operator *= (const vec3& other)`)
- Multiplies the components of this vec3 with `other`, which is functionally equivalent to the statement `myVec3 = myVec3 * myOtherVec3;`.

Divide-equal (`void operator /= (const vec3& other)`)
- Divides the components of this vec3 by `other`, which is functionally equivalent to the statement `myVec3 = myVec3 / myOtherVec3`.
- Like the non-assigning division operator, this one also includes safety checks to prevent division-by-zero errors. See `unsafeDivideBy()` for a version without these checks.

##### Logical

Equality (`bool operator == (const vec3& other)`)
- Checks each component of this vec3 against `other` to determine if the entire vec3 is equal to `other` or not.
- Returns false if one or more of this vec3's components is not equal to that of `other`, and returns true otherwise.

Inequality (`bool operator != (const vec3& other)`)
- Checks each component of this vec3 against `other` to determine if the entire vec3 is unequal to `other` or not - this behaves similarly to the equality operator, but gives the opposite result.
- Returns true if one or more of this vec3's components is not equal to that of `other`, and returns false otherwise.

### Functions

##### Scalar

`vec3 multipliedByFloat(float in) const`
- Returns a new vec3 created by multiplying each component of this vec3 by the single value `in`. This is the non-assigning version; for the assigning version, see `multiplyByFloat()`.
- This serves more of a convenience purpose than a functional one. If this vec3 has a total length of 1, for example, you can use this function to grow or shrink it to the length of `in`.

`void multiplyByFloat(float in)`
- Multiplies each component of this vec3 by the single value `in`. This is the assigning version; for the non-assigning version, see `multipliedByFloat()`.

`vec3 dividedByFloat(float in) const`
- Returns a new vec3 created by dividing each component of this vec3 by the single value `in`.
- This function has safety checks built in to prevent division-by-zero errors. However, unlike the previous safe division functions, which check against each component of the other vec3, this function only checks the value of `in` once. Therefore, if you're thinking about using the `/` operator in the case where every component of the other vec3 has the same value, you might want to consider this instead for an efficiency boost. Note that this does not apply to `unsafeDividedByFloat()`, which has no safety checks at all.

`void divideByFloat(float in)`
- Divides each component of this vec3 by the single value `in`.
- This function's safety checks behave the same as in `dividedByFloat()`. Therefore, if you're thinking about using the `/=` operator in the case where every component of the other vec3 has the same value, you might want to consider this instead for an efficiency boost. This does not apply to `unsafeDivideByFloat()`, which has no safety checks at all.

##### Advanced math

`vec3 lerpedTo(const vec3& other, float amount) const`
- Returns a new vec3 created by linearly interpolating between the components of this vec3 and `other` by the factor `amount`, using the formula `this + ((other - this) * amount)`. This is the non-assigning version; for the assigning version, see `lerpTo()`.

`void lerpTo(const vec3& other, float amount)`
- Linearly interpolates the components of this vec3 with `other` by the factor `amount`, using the formula `this += ((other - this) * amount)`. This is the assigning version; for the non-assigning version, see `lerpedTo()`.

`float getLength() const`
- Returns the length of a straight line spanning from `{ 0.0f, 0.0f, 0.0f }` to the coordinates of this vec3 using the Pythagorean theorem, effectively getting the coordinates' distance from a point at `{ 0.0f }`.
- To get the length between two vec3s `A` and `B`, use `getLength(B - A)`.

`vec3 normalized() const`
- Returns a new vec3 created by dividing this vec3's components by its length, effectively shrinking or growing its length to 1 while preserving its direction (converting it to what is called a "unit vector"). This is the non-assigning version; for the assigning version, see `normalize()`.
- This function includes safety checks with the same behavior as `dividedByFloat()`. For a version with no safety checks, see `unsafeNormalized()`.

`void normalize()`
- Divides this vec3's components by its length, converting it to a unit vector. This is the assigning version; for the non-assigning version, see `normalized()`.

`float getDotProduct(const vec3& other) const`
- **Note: This function assumes that both this vec3 and `other` are already unit vectors when passed into it. Make sure that both of them are normalized (having a length of 1.0f) beforehand for correct behavior!**
- Calculates the dot product between this vec3 and `other` using the formula `(x * other.x) + (y * other.y) + ...`, which has the effect of calculating how closely the direction of this vec3 and `other` are aligned, assuming that both of them are unit vectors. Returns 1.0f if both vec3s are perfectly aligned, -1.0f if they oppose each other, and 0.0f if they are perpendicular (forming a 90° angle).

##### Unsafe division

```
DISCLAIMER: These functions do not contain the safety checks seen in the / and /= operators, dividedByFloat(), divideByFloat(), normalized() and normalize(). This could potentially give them a performance benefit, but they can be dangerous since they do nothing to prevent dividing by zero. This can result in unexpected results, exceptions being thrown, or other undefined behavior. Make sure that the parameters "const vec3& other" or "float in" or any of their values will never reach 0.0f to prevent this from happening!
```

`vec3 unsafeDividedBy(const vec3& other) const`
- The unsafe equivalent of `operator /`. Returns a new vec3 created by dividing the components of this vec3 by `other` while skipping the safety checks seen in `operator /`.
- This is the non-assigning version; for the assigning version, see `unsafeDivideBy()`.

`vec3 unsafeDividedByFloat(float in) const`
- The unsafe equivalent of `dividedByFloat()`. Returns a new vec3 created by dividing each component of this vec3 by `in` while skipping the safety check seen in `dividedByFloat()`.
- This is the non-assigning version; for the assigning version, see `unsafeDivideByFloat()`.

`void unsafeDivideBy(const vec3& other)`
- The unsafe equivalent of `operator /=`. Divides the components of this vec3 by `other` while skipping the safety checks seen in `operator /=`.
- This is the assigning version; for the non-assigning version, see `unsafeDividedBy()`.

`void unsafeDivideByFloat(float in)`
- The unsafe equivalent of `divideByFloat()`. Divides each component of this vec3 by `in` while skipping the safety check seen in `divideByFloat()`.
- This is the assigning version; for the non-assigning version, see `unsafeDividedByFloat()`.

`vec3 unsafeNormalized() const`
- The unsafe equivalent of `normalized()`. Returns a new vec3 created by dividing this vec3's components by its length, converting it to a unit vector. This is the non-assigning version; for the assigning version, see `unsafeNormalize()`.

`void unsafeNormalize()`
- The unsafe equivalent of `normalize()`. Divides this vec3's components by its length, converting it to a unit vector. This is the assigning version; for the non-assigning version, see `unsafeNormalized()`.

### Member variables

`float x`
- The X component of the vec3. Initialized to zero.

`float y`
- The Y component of the vec3. Initialized to zero.

`float z`
- The Z component of the vec3. Initialized to zero.

### Remarks

Pay close attention to the difference in naming between `multipliedByFloat()` and `multiplyByFloat()`, and other functions like this. The former creates a new vec3 from the current one, while the latter directly modifies the current vec3.

Along with vec2, the entire vec3 type doesn't have SIMD implemented right now, aside from autovectorization which may be performed at compile time. vec4 is the only type with a manual SIMD implementation for some functions.

Historically, the vec3 struct (along with vec2 and vec4) had a helper function called `getSize()`. This was a static function (this means that it behaved like a global function while only being accessible from the class namespace) which returned the size of two, three or four floats by using the `sizeof()` operator and bit shifting. For vec2, it returned `sizeof(float) << 1`, for vec3 it returned `(sizeof(float) << 1) + sizeof(float)`, and for vec4 it returned `sizeof(float) << 2`. I don't remember why I implemented it, maybe I was just using it as an excuse to try out bit shifting as opposed to regular multiplication by 2/3/4, but these functions were removed once I found out that `sizeof()` is a _COMPILE-TIME_ OPERATOR! Calls to `sizeof(structureType)` are replaced with the size of `structureType` by the compiler, which means that it has literally zero runtime overhead. The good-for-nothing `getSize()` functions added a tiny amount of unnecessary overhead which I thought I was avoiding, and they were also used nowhere in ibex3D anyway, so I removed them as a result. This entire paragraph might not be necessary (it's more like a rant), but I felt like putting it here as a note-to-self that `sizeof()` is a compile-time operator and that it has no runtime cost or performance penalty.

### Examples

Using the vec3 constructors and accessing its components:
```cpp
// Init-on-declaration
vec3 myVec3_noConstructor;                              // values: { 0.0f, 0.0f, 0.0f }
vec3 myVec3_defaultConstructor();                       // values: { 0.0f, 0.0f, 0.0f }
vec3 myVec3_mainConstructor(0.0f, 1.0f, 2.0f);          // values: { 0.0f, 1.0f, 2.0f }
vec3 myVec3_singleConstructor(4.0f);                    // values: { 4.0f, 4.0f, 4.0f }

// Init-after-declaration using a new vec3
myVec3_noConstructor = vec3(0.0f, 1.0f, 2.0f);          // values: { 0.0f, 1.0f, 2.0f }

// Init-after-declaration using an existing vec3
myVec3_defaultConstructor = myVec3_noConstructor;       // values: { 0.0f, 1.0f, 2.0f }

// Per-component writing
myVec3_singleConstructor.x = 0.0f;                      // values: { 0.0f, 4.0f, 4.0f }
myVec3_singleConstructor.y = 3.0f;                      // values: { 0.0f, 3.0f, 4.0f }
myVec3_singleConstructor.z = 6.0f;                      // values: { 0.0f, 3.0f, 6.0f }

// Per-component reading
float myFloat = myVec3_singleConstructor.x;             // value: 0.0f
myFloat = myVec3_singleConstructor.y;                   // value: 3.0f
myFloat = myVec3_singleConstructor.z;                   // value: 6.0f
```

Arithmetic, arithmetic-assignment and logical operators:
```cpp
// Addition - values: { 4.0f, 6.0f, 8.0f }
vec3 myVec3 = vec3(0.0f, 1.0f, 2.0f) + vec3(4.0f, 5.0f, 6.0f);

// Subtraction - values: { -1.0f, -1.0f, -1.0f }
myVec3 = vec3(4.0f, 5.0f, 6.0f) - vec3(5.0f, 6.0f, 7.0f);

// Multiplication - values: { 4.0f, 8.0f, 16.0f }
myVec3 = vec3(1.0f, 2.0f, 4.0f) * vec3(4.0f);

// Division (valid) - values: { 1.0f, 2.0f, 4.0f }
myVec3 = vec3(4.0f, 8.0f, 16.0f) / vec3(4.0f);

// Division by zero - values: { 0.0f, 0.0f, 0.0f }
myVec3 = vec3(4.0f, 8.0f, 16.0f) / vec3(4.0f, 4.0f, 0.0f);

// Add-assignment - values: { 2.0f, 2.0f, 2.0f }
myVec3 += vec3(2.0f);

// Subtract-assignment - values: { 1.0f, 1.0f, 1.0f }
myVec3 -= vec3(1.0f);

// Multiply-assignment - values: { 4.0f, 4.0f, 4.0f }
myVec3 *= vec3(4.0f);

// Divide-assignment (valid) - values: { 8.0f, 8.0f, 0.5f }
myVec3 /= vec3(0.5f, 0.5f, 8.0f);

// Divide-assignment by zero - values: { 0.0f, 0.0f, 0.0f }
myVec3 /= vec3(0.5f, 0.5f, 0.0f);

// Equality - value: true
bool myBool = myVec3 == myVec3;

// Inequality - value: false
myBool = myVec3 != myVec3;
```

Scalar functions:
```cpp
// Multiplication by float - values: { 2.0f, 0.0f, 2.0f }
vec3 myVec3 = vec3(1.0f, 0.0f, 1.0f).multipliedByFloat(2.0f);

// Multiply-assignment by float - values: { 6.0f, 0.0f, 6.0f }
myVec3.multiplyByFloat(3.0f);

// Division by float (valid) - values: { 1.0f, 0.0f, 1.0f }
myVec3 = myVec3.dividedByFloat(6.0f);

// Divide-assignment by float (valid) - values: { 0.5f, 0.0f, 0.5f }
myVec3.divideByFloat(2.0f);

// Division by zero (float) - values: { 0.0f, 0.0f, 0.0f }
myVec3 = myVec3.dividedByFloat(0.0f):

// Divide-assignment by zero (float) - values: { 0.0f, 0.0f, 0.0f }
myVec3.divideByFloat(0.0f);
```

Advanced math functions:
```cpp
// Linear interpolation (non-assigning) - values: { 0.0f, 0.5f, 1.0f }
vec3 myVec3 = vec3(0.0f, 1.0f, 2.0f).lerpedTo(vec3(0.0f), 0.5f);

// Linear interpolation (assigning) - values: { 0.5f, 0.75f, 1.0f }
myVec3.lerpTo(vec3(1.0f), 0.5f);

// Length and normalization (non-assigning)
myVec3 = vec3(3.0f, 4.0f, 0.0f);                    // length: 5.0f
float myVec3Length = myVec3.getLength();
myVec3 = myVec3.normalized();                       // values: { 0.6f, 0.8f, 0.0f }, length: 1.0f
myVec3 = myVec3.multipliedByFloat(myVec3Length);    // values: { 3.0f, 4.0f, 0.0f }, length: 5.0f

// Length and normalization (assigning)
myVec3 = vec3(3.0f, 4.0f, 0.0f);                    // length: 5.0f
myVec3Length = myVec3.getLength();
myVec3.normalize();                                 // values: { 0.6f, 0.8f, 0.0f }, length: 1.0f
myVec3.multiplyByFloat(myVec3Length);               // values: { 3.0f, 4.0f, 0.0f }, length: 5.0f

// Dot product (aligned) - value: 1.0f
float myDotProduct = vec3(1.0f, 0.0f, 0.0f).getDotProduct(vec3(1.0f, 0.0f, 0.0f));

// Dot product (opposing) - value: -1.0f
myDotProduct = vec3(1.0f, 0.0f, 0.0f).getDotProduct(vec3(-1.0f, 0.0f, 0.0f));

// Dot product (perpendicular) - value: 0.0f
myDotProduct = vec3(1.0f, 0.0f, 0.0f).getDotProduct(vec3(0.0f, 1.0f, 0.0f));
```

### To-do list

- Figure out how to implement SIMD for this type along with vec2.