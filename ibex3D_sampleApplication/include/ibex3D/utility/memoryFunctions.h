#pragma once

#include <new>			// Needed for the placement new operator
#include <stdio.h>		// Needed for printf
#include <stdlib.h>		// Needed for malloc and free
#include <utility>		// Needed for std::forward

struct memoryStatistics_t
{
	size_t totalUsage = 0;
};

static memoryStatistics_t memoryStatistics;

template<typename T>
T* ibex3D_alloc()
{
	size_t memSize = sizeof(T);
	memoryStatistics.totalUsage += memSize;
	
	printf("Allocated %zu bytes of memory. (Now using %zu)\n", memSize, memoryStatistics.totalUsage);

	void* ptr = malloc(memSize);
	return new(ptr) T;
}

template<typename T, typename U>
T* ibex3D_alloc(U&& args)
{
	size_t memSize = sizeof(T);
	memoryStatistics.totalUsage += memSize;
	
	printf("Allocated %zu bytes of memory. (Now using %zu)\n", memSize, memoryStatistics.totalUsage);

	void* ptr = malloc(sizeof(T));
	return new(ptr) T(std::forward<U>(args));
}

template<typename T>
void ibex3D_free(T* mem)
{
	size_t memSize = sizeof(T);
	memoryStatistics.totalUsage -= memSize;

	printf("Freed %zu bytes of memory. (Now using %zu)\n", memSize, memoryStatistics.totalUsage);

	mem -> ~T();
	free(reinterpret_cast<void*>(mem));
}