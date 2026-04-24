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
    - [Extras](#extras)
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
- An overloaded constructor which allows you to set every component of the vec4 to the single float parameter passed in. For example, if you were to say `vec4 myVec4(3.0f)` or `vec4 myVec4 = vec4(3.0f)`, `myVec4` would have the values `{ x=3.0f, y=3.0f, z=3.0f, w=3.0f }`.

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

`void multiplyByFloat(float in)`

`vec4 dividedByFloat(float in) const`

`void divideByFloat(float in)`

##### Advanced math

`vec4 lerpedTo(const vec4& other, float amount) const`

`void lerpTo(const vec4& other, float amount)`

`float getLength() const`

`vec4 normalized() const`

`void normalize()`

`float getDotProduct(const vec4& other) const`

##### Unsafe division

`vec4 unsafeDividedBy(const vec4& other) const`

`vec4 unsafeDividedByFloat(float in) const`

`void unsafeDivideBy(const vec4& other)`

`void unsafeDivideByFloat(float in)`

`vec4 unsafeNormalized() const`

`void unsafeNormalize()`

##### Extras

`static size_t getSize()`

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

### Examples

How to (do something with) a className instance (as used in the file `path/to/source_code_file.cpp`)
```cpp
Paste some C++ source code demonstrating how the class should be used. If this is a GLSL file, replace the "cpp" at the top of the very beginning of the code block with "glsl".
```

### To-do list

- Try implementing non-assigning functions with the SSE instruction set again, but build the program with Clang and GCC to see if the slower speed isn't an MSVC-specific issue.