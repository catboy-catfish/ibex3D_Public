#include <ibex3D/core/entryPoint.h>

#include <ibex3D/math/vec4.h>
#include <ibex3D/math/vec4_simd.h>

#include <chrono>
#include <stdio.h>

#define NUM_LOOPS 100000000

int ibex3D_entryPoint()
{	
	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point endTime;
	float duration;

	// ----------------------------------------------------------------------------------------------------

	vec4 myVec0 = { 0.0f, 1.0f, 2.0f, 3.0f };
	vec4 myVec1 = { 4.0f, 5.0f, 6.0f, 9.0f };
	vec4 myVec2;

	startTime = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i < NUM_LOOPS; i++)
	{
		myVec2 = myVec0 + myVec1;
	}
	
	endTime = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration<float>(endTime - startTime).count();

	fprintf(stdout, "Addition without SIMD took %f seconds to run %d times.\n", duration, NUM_LOOPS);
	fprintf(stdout, "Result = { %f, %f, %f, %f }\n\n", myVec2.x, myVec2.y, myVec2.z, myVec2.w);

	// ----------------------------------------------------------------------------------------------------

	vec4_simd mySimdVec0 = { 0.0f, 1.0f, 2.0f, 3.0f };
	vec4_simd mySimdVec1 = { 4.0f, 5.0f, 6.0f, 9.0f };
	vec4_simd mySimdVec2;

	startTime = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i < NUM_LOOPS; i++)
	{
		mySimdVec2 = mySimdVec0 + mySimdVec1;
	}

	endTime = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration<float>(endTime - startTime).count();

	fprintf(stdout, "Addition with SIMD took %f seconds to run %d times.\n", duration, NUM_LOOPS);
	fprintf(stdout, "Result = { %f, %f, %f, %f }\n\n", mySimdVec2.f[0], mySimdVec2.f[1], mySimdVec2.f[2], mySimdVec2.f[3]);

	// ----------------------------------------------------------------------------------------------------

	return 0;
}