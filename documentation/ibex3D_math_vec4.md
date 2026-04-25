# className - Basic overview

* Header file: `include/ibex3D/math/vec4.h`
* Source file: `source/ibex3D/math/vec4.h`

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

A simple struct which contains four single-precision floats bundled together, and also includes overrides/functions for various mathematical operations.

### Constructors

`vec4() = default`
- The default constructor (with no parameters). This leaves all coordinates of the vec4 with a value of zero.
- The reason why this entire declaration is in place is because when an overloaded constructor (with parameters) exists, the compiler assumes that that is the default constructor and refuses to create an extra parameter-less constructor by default, making it impossible to create a vec4 without specifying its values. Declaring `vec4() = default;` is effectively telling the compiler "Actually, no, _this_ is the default constructor" and makes it possible to create a vec4 again without having to specify its values.

`vec4(float inX, float inY, float inZ, float inW) : x(inX), y(inY), z(inZ), w(inW) {}`
- An overloaded constructor which allows you to manually specify the X, Y, Z and W components of the vec4.
- The syntax `x(inX), y(inY), ...` automatically assigns the value of the parameter `inX` to the member variable `x`, assigns the value of `inY` to `y`, and so on. This is functionally equivalent to writing `{ x = inX; y = inY; ... }` in the constructor body (the brackets at the end).

`vec4(float inXYZW) : x(inXYZW), y(inXYZW), z(inXYZW), w(inXYZW) {}`
- An overloaded constructor which allows you to set every component of the vec4 to the single float parameter passed in. For example, if you were to say `vec4 myVec4(3.0f)`, `myVec4` would have the values `{ x=3.0f, y=3.0f, z=3.0f, w=3.0f }`.

### Operators

##### Arithmetic

```
Disclaimer: These functions don't have SIMD implemented right now. I do want to do it in order to improve performance, but I cannot right now, and I put a detailed explanation as to why in the Remarks.
```

Addition (`vec4 operator + (const vec4& other) const`)
- Returns a new vec4 created from the components of this vec4 and `other` added together. This doesn't modify the values of this vec4 or `other`. 
- The argument `const vec4& other` is used instead of just `vec4 other` because the former passes `other` by reference, while the latter creates an unwanted copy of `other` when it's passed to the operator. In the case of `vec4`, it's very small, but I feel like it's still good practice in functions like this.
- The `const` qualifier at the start of the parameter `other` prevents the code in the function body from modifying it. `const` is used again at the end of the function declaration, and this time it means the code in the function body cannot modify its own variables. It's only natural to do this in this kind of function because it does neither of these.

Subtraction (`vec4 operator - (const vec4& other) const`)
- Returns a new vec4 created from the components of `other` subtracted from this vec4. This doesn't modify the values of this vec4 or `other`.

Multiplication (`vec4 operator * (const vec4& other) const`)
- Returns a new vec4 created from the components of this vec4 and `other` multiplied together. This doesn't modify the values of this vec4 or `other`.

Division (`vec4 operator / (const vec4& other) const`)
- Returns a new vec4 created from the components of this vec4 divided by `other`. This doesn't modify the values of this vec4 or `other`.
- This operator includes some safety checks which cause it to return `vec4(0.0f)` if any of `other`'s components are 0 to prevent division-by-zero errors. If you need a version of this operator without these checks, see `unsafeDividedBy()`.

##### Arithmetic assignment

Add-equal (`void operator += (const vec4& other)`)
- Adds the components of `other` to this vec4, which is functionally equivalent to the statement `myVec4 = myVec4 + myOtherVec4;`. This function doesn't create a new vec4, it directly modifies the current vec4, which is why it doesn't have any return value.
- The `const` qualifier at the end of the declaration has been removed because it now modifies its own variables.

Subtract-equal (`void operator -= (const vec4& other)`)
- Subtracts the components of `other` from this vec4, which is functionally equivalent to the statement `myVec4 = myVec4 - myOtherVec4;`.

Multiply-equal (`void operator *= (const vec4& other)`)
- Multiplies the components of this vec4 with `other`, which is functionally equivalent to the statement `myVec4 = myVec4 * myOtherVec4;`.

Divide-equal (`void operator /= (const vec4& other)`)
- Divides the components of this vec4 by `other`, which is functionally equivalent to the statement `myVec4 = myVec4 / myOtherVec4`.
- Like the non-assigning division operator, this one also includes safety checks to prevent division-by-zero errors. See `unsafeDivideBy()` for a version without these checks.

##### Logical

Equality (`bool operator == (const vec4& other)`)
- Checks each component of this vec4 against `other` to determine if the entire vec4 is equal to `other` or not.
- Returns false if one or more of this vec4's components is not equal to that of `other`, and returns true otherwise.

Inequality (`bool operator != (const vec4& other)`)
- Checks each component of this vec4 against `other` to determine if the entire vec4 is unequal to `other` or not - this behaves similarly to the equality operator, but gives the opposite result.
- Returns true if one or more of this vec4's components is not equal to that of `other`, and returns false otherwise.

### Functions

##### Scalar

`vec4 multipliedByFloat(float in) const`
- Returns a new vec4 created by multiplying each component of this vec4 by the single value `in`. This is the non-assigning version; for the assigning version, see `multiplyByFloat()`.
- This serves more of a convenience purpose than a functional one. If this vec4 has a total length of 1, for example, you can use this function to grow or shrink it to the length of `in`.

`void multiplyByFloat(float in)`
- Multiplies each component of this vec4 by the single value `in`. This is the assigning version; for the non-assigning version, see `multipliedByFloat()`.

`vec4 dividedByFloat(float in) const`
- Returns a new vec4 created by dividing each component of this vec4 by the single value `in`.
- This function has safety checks built in to prevent division-by-zero errors. However, unlike the previous safe division functions, which check against each component of the other vec4, this function only checks the value of `in` once. Therefore, if you're thinking about using the `/` operator in the case where every component of the other vec4 has the same value, you might want to consider this instead for an efficiency boost. Note that this does not apply to `unsafeDividedByFloat()`, which has no safety checks at all.

`void divideByFloat(float in)`
- Divides each component of this vec4 by the single value `in`.
- This function's safety checks behave the same as in `dividedByFloat()`. Therefore, if you're thinking about using the `/=` operator in the case where every component of the other vec4 has the same value, you might want to consider this instead for an efficiency boost. This does not apply to `unsafeDivideByFloat()`, which has no safety checks at all.

##### Advanced math

`vec4 lerpedTo(const vec4& other, float amount) const`
- Returns a new vec4 created by linearly interpolating between the components of this vec4 and `other` by the factor `amount`, using the formula `this + ((other - this) * amount)`. This is the non-assigning version; for the assigning version, see `lerpTo()`.

`void lerpTo(const vec4& other, float amount)`
- Linearly interpolates the components of this vec4 with `other` by the factor `amount`, using the formula `this += ((other - this) * amount)`. This is the assigning version; for the non-assigning version, see `lerpedTo()`.

`float getLength() const`
- Returns the length of a straight line spanning from `{ 0.0f, 0.0f, 0.0f, 0.0f }` to the coordinates of this vec4 using the Pythagorean theorem, effectively getting the coordinates' distance from a point at `{ 0.0f }`.
- To get the length between two vec4s `A` and `B`, use `getLength(B - A)`.

`vec4 normalized() const`
- Returns a new vec4 created by dividing this vec4's components by its length, effectively shrinking or growing its length to 1 while preserving its direction (converting it to what is called a "unit vector"). This is the non-assigning version; for the assigning version, see `normalize()`.
- This function includes safety checks with the same behavior as `dividedByFloat()`. For a version with no safety checks, see `unsafeNormalized()`.

`void normalize()`
- Divides this vec4's components by its length, converting it to a unit vector. This is the assigning version; for the non-assigning version, see `normalized()`.

`float getDotProduct(const vec4& other) const`
- **Note: This function assumes that both this vec4 and `other` are already unit vectors when passed into it. Make sure that both of them are normalized (having a length of 1.0f) beforehand for correct behavior!**
- Calculates the dot product between this vec4 and `other` using the formula `(x * other.x) + (y * other.y) + ...`, which has the effect of calculating how closely the direction of this vec4 and `other` are aligned, assuming that both of them are unit vectors. Returns 1.0f if both vec4s are perfectly aligned, -1.0f if they oppose each other, and 0.0f if they are perpendicular (forming a 90° angle).

##### Unsafe division

```
DISCLAIMER: These functions do not contain the safety checks seen in the / and /= operators, dividedByFloat(), divideByFloat(), normalized() and normalize(). This could potentially give them a performance benefit, but they can be dangerous since they do nothing to prevent dividing by zero. This can result in unexpected results, exceptions being thrown, or other undefined behavior. Make sure that the parameters "const vec4& other" or "float in" or any of their values will never reach 0.0f to prevent this from happening!
```

`vec4 unsafeDividedBy(const vec4& other) const`
- The unsafe equivalent of `operator /`. Returns a new vec4 created by dividing the components of this vec4 by `other` while skipping the safety checks seen in `operator /`.
- This is the non-assigning version; for the assigning version, see `unsafeDivideBy()`.

`vec4 unsafeDividedByFloat(float in) const`
- The unsafe equivalent of `dividedByFloat()`. Returns a new vec4 created by dividing each component of this vec4 by `in` while skipping the safety check seen in `dividedByFloat()`.
- This is the non-assigning version; for the assigning version, see `unsafeDivideByFloat()`.

`void unsafeDivideBy(const vec4& other)`
- The unsafe equivalent of `operator /=`. Divides the components of this vec4 by `other` while skipping the safety checks seen in `operator /=`.
- This is the assigning version; for the non-assigning version, see `unsafeDividedBy()`.

`void unsafeDivideByFloat(float in)`
- The unsafe equivalent of `divideByFloat()`. Divides each component of this vec4 by `in` while skipping the safety check seen in `divideByFloat()`.
- This is the assigning version; for the non-assigning version, see `unsafeDividedByFloat()`.

`vec4 unsafeNormalized() const`
- The unsafe equivalent of `normalized()`. Returns a new vec4 created by dividing this vec4's components by its length, converting it to a unit vector. This is the non-assigning version; for the assigning version, see `unsafeNormalize()`.

`void unsafeNormalize()`
- The unsafe equivalent of `normalize()`. Divides this vec4's components by its length, converting it to a unit vector. This is the assigning version; for the non-assigning version, see `unsafeNormalized()`.

### Member variables

`float x`
- The X component of the vec4. Initialized to zero.

`float y`
- The Y component of the vec4. Initialized to zero.

`float z`
- The Z component of the vec4. Initialized to zero.

`float w`
- The W component of the vec4. Initialized to zero.

### Remarks

Pay close attention to the difference in naming between `multipliedByFloat()` and `multiplyByFloat()`, and other functions like this. The former creates a new vec4 from the current one, while the latter directly modifies the current vec4.

The non-assigning arithmetic operators (`+ - * /`), don't have SIMD implemented right now. The reason why is pretty strange: when I tried to implement non-assignment functions using the SSE instruction set and benchmarked them against SISD equivalent functions, the SIMD functions ended up being _slower_ than the SISD functions. However, when I implemented assigning arithmetic functions (`+= -= *= /=`) using SSE and compared them with the SISD equivalents, they ended up being faster, easier to implement, and the logic remains in the vec4 code to this day. I'm not yet sure why this is, although I've only tested it using the MSVC compiler - I should try doing this again using Clang and GCC to see if it's not a compiler-specific issue. I very much want to speed these operators up using SIMD, but I don't know how to do so yet, if it's even possible.

Historically, the vec4 struct (along with vec2 and vec3) had a helper function called `getSize()`. This was a static function (this means that it behaved like a global function while only being accessible from the class namespace) which returned the size of two, three or four floats by using the `sizeof()` operator and bit shifting. For vec2, it returned `sizeof(float) << 1`, for vec3 it returned `(sizeof(float) << 1) + sizeof(float)`, and for vec4 it returned `sizeof(float) << 2`. I don't remember why I implemented it, maybe I was just using it as an excuse to try out bit shifting as opposed to regular multiplication by 2/3/4, but these functions were removed once I found out that `sizeof()` is a _COMPILE-TIME_ OPERATOR! Calls to `sizeof(structureType)` are replaced with the size of `structureType` by the compiler, which means that it has literally zero runtime overhead. The good-for-nothing `getSize()` functions added a tiny amount of unnecessary overhead which I thought I was avoiding, and they were also used nowhere in ibex3D anyway, so I removed them as a result. This entire paragraph might not be necessary (it's more like a rant), but I felt like putting it here as a note-to-self that `sizeof()` is a compile-time operator and that it has no runtime cost or performance penalty.

### Examples

Using the vec4 constructors and accessing its components:
```cpp
// Init-on-declaration
vec4 myVec4_noConstructor;                              // values: { 0.0f, 0.0f, 0.0f, 0.0f }
vec4 myVec4_defaultConstructor();                       // values: { 0.0f, 0.0f, 0.0f, 0.0f }
vec4 myVec4_mainConstructor(0.0f, 1.0f, 2.0f, 3.0f);    // values: { 0.0f, 1.0f, 2.0f, 3.0f }
vec4 myVec4_singleConstructor(4.0f);                    // values: { 4.0f, 4.0f, 4.0f, 4.0f }

// Init-after-declaration using a new vec4
myVec4_noConstructor = vec4(0.0f, 1.0f, 2.0f, 3.0f);    // values: { 0.0f, 1.0f, 2.0f, 3.0f }

// Init-after-declaration using an existing vec4
myVec4_defaultConstructor = myVec4_noConstructor;       // values: { 0.0f, 1.0f, 2.0f, 3.0f }

// Per-component writing
myVec4_singleConstructor.x = 0.0f;                      // values: { 0.0f, 4.0f, 4.0f, 4.0f }
myVec4_singleConstructor.y = 3.0f;                      // values: { 0.0f, 3.0f, 4.0f, 4.0f }
myVec4_singleConstructor.z = 6.0f;                      // values: { 0.0f, 3.0f, 6.0f, 4.0f }
myVec4_singleConstructor.w = 7.0f;                      // values: { 0.0f, 3.0f, 6.0f, 7.0f }

// Per-component reading
float myFloat = myVec4_singleConstructor.x;             // value: 0.0f
myFloat = myVec4_singleConstructor.y;                   // value: 3.0f
myFloat = myVec4_singleConstructor.z;                   // value: 6.0f
myFloat = myVec4_singleConstructor.w;                   // value: 7.0f
```

Arithmetic, arithmetic-assignment and logical operators:
```cpp
// Addition - values: { 4.0f, 6.0f, 8.0f, 10.0f }
vec4 myVec4 = vec4(0.0f, 1.0f, 2.0f, 3.0f) + vec4(4.0f, 5.0f, 6.0f, 7.0f);

// Subtraction - values: { -1.0f, -1.0f, -1.0f, -1.0f }
myVec4 = vec4(4.0f, 5.0f, 6.0f, 7.0f) - vec4(5.0f, 6.0f, 7.0f, 8.0f);

// Multiplication - values: { 4.0f, 8.0f, 16.0f, 32.0f }
myVec4 = vec4(1.0f, 2.0f, 4.0f, 8.0f) * vec4(4.0f);

// Division (valid) - values: { 1.0f, 2.0f, 4.0f, 8.0f }
myVec4 = vec4(4.0f, 8.0f, 16.0f, 32.0f) / vec4(4.0f);

// Division by zero - values: { 0.0f, 0.0f, 0.0f, 0.0f }
myVec4 = vec4(4.0f, 8.0f, 16.0f, 32.0f) / vec4(4.0f, 4.0f, 0.0f, 4.0f);

// Add-assignment - values: { 2.0f, 2.0f, 2.0f, 2.0f }
myVec4 += vec4(2.0f);

// Subtract-assignment - values: { 1.0f, 1.0f, 1.0f, 1.0f }
myVec4 -= vec4(1.0f);

// Multiply-assignment - values: { 4.0f, 4.0f, 4.0f, 4.0f }
myVec4 *= vec4(4.0f);

// Divide-assignment (valid) - values: { 8.0f, 8.0f, 0.5f, 8.0f }
myVec4 /= vec4(0.5f, 0.5f, 8.0f, 0.5f);

// Divide-assignment by zero - values: { 0.0f, 0.0f, 0.0f, 0.0f }
myVec4 /= vec4(0.5f, 0.5f, 0.0f, 0.5f);

// Equality - value: true
bool myBool = myVec4 == myVec4;

// Inequality - value: false
myBool = myVec4 != myVec4;
```

Scalar functions:
```cpp
// Multiplication by float - values: { 2.0f, 0.0f, 2.0f, 4.0f }
vec4 myVec4 = vec4(1.0f, 0.0f, 1.0f, 2.0f).multipliedByFloat(2.0f);

// Multiply-assignment by float - values: { 6.0f, 0.0f, 6.0f, 12.0f }
myVec4.multiplyByFloat(3.0f);

// Division by float (valid) - values: { 1.0f, 0.0f, 1.0f, 2.0f }
myVec4 = myVec4.dividedByFloat(6.0f);

// Divide-assignment by float (valid) - values: { 0.5f, 0.0f, 0.5f, 1.0f }
myVec4.divideByFloat(2.0f);

// Division by zero (float) - values: { 0.0f, 0.0f, 0.0f, 0.0f }
myVec4 = myVec4.dividedByFloat(0.0f):

// Divide-assignment by zero (float) - values: { 0.0f, 0.0f, 0.0f, 0.0f }
myVec4.divideByFloat(0.0f);
```

Advanced math functions:
```cpp
// Linear interpolation (non-assigning) - values: { 0.0f, 0.5f, 1.0f, 1.5f }
vec4 myVec4 = vec4(0.0f, 1.0f, 2.0f, 3.0f).lerpedTo(vec4(0.0f), 0.5f);

// Linear interpolation (assigning) - values: { 0.5f, 0.75f. 1.0f, 1.25f }
myVec4.lerpTo(vec4(1.0f), 0.5f);

// Length and normalization (non-assigning)
myVec4 = vec4(3.0f, 4.0f, 0.0f, 0.0f);              // length: 5.0f
float myVec4Length = myVec4.getLength();
myVec4 = myVec4.normalized();                       // values: { 0.6f, 0.8f, 0.0f, 0.0f }, length: 1.0f
myVec4 = myVec4.multipliedByFloat(myVec4Length);    // values: { 3.0f, 4.0f, 0.0f, 0.0f }, length: 5.0f

// Length and normalization (assigning)
myVec4 = vec4(3.0f, 4.0f, 0.0f, 0.0f);              // length: 5.0f
myVec4Length = myVec4.getLength();
myVec4.normalize();                                 // values: { 0.6f, 0.8f, 0.0f, 0.0f }, length: 1.0f
myVec4.multiplyByFloat(myVec4Length);               // values: { 3.0f, 4.0f, 0.0f, 0.0f }, length: 5.0f

// Dot product (aligned) - value: 1.0f
float myDotProduct = vec4(1.0f, 0.0f, 0.0f, 0.0f).getDotProduct(vec4(1.0f, 0.0f, 0.0f, 0.0f));

// Dot product (opposing) - value: -1.0f
myDotProduct = vec4(1.0f, 0.0f, 0.0f, 0.0f).getDotProduct(vec4(-1.0f, 0.0f, 0.0f, 0.0f));

// Dot product (perpendicular) - value: 0.0f
myDotProduct = vec4(1.0f, 0.0f, 0.0f, 0.0f).getDotProduct(vec4(0.0f, 1.0f, 0.0f, 0.0f));
```

### To-do list

- Try implementing non-assigning functions with the SSE instruction set again, but build the program with Clang and GCC to see if the slower speed isn't an MSVC-specific issue.