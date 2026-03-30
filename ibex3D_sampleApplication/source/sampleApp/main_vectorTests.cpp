#include <ibex3D/math/vec4.h>

int main()
{
	vec4 myVector(0.0f, 1.0f, 2.0f, 3.0f);
	vec4 myVector2(4.0f, 5.0f, 6.0f, 7.0f);
	
	myVector /= myVector2;

	return 0;
}