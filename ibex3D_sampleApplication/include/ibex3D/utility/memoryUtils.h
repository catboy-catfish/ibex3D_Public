#pragma once

#include <stdio.h>
#include <stdlib.h>

struct memoryStats_t
{
	size_t totalUsage = 0;
};

static memoryStats_t memoryStats;

void* operator new(size_t size)
{
	memoryStats.totalUsage += size;
	printf("Allocated %zu bytes (now using %zu bytes).\n", size, memoryStats.totalUsage);

	return malloc(size);
}

void operator delete(void* memory, size_t size)
{
	memoryStats.totalUsage -= size;
	printf("Deallocated %zu bytes (now using %zu bytes).\n", size, memoryStats.totalUsage);

	free(memory);
}