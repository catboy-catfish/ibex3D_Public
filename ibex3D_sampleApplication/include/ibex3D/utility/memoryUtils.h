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
	printf("Allocated %d bytes (now using %d bytes).\n", (int)size, (int)memoryStats.totalUsage);

	return malloc(size);
}

void operator delete(void* memory, size_t size)
{
	memoryStats.totalUsage -= size;
	printf("Deallocated %d bytes (now using %d bytes).\n", (int)size, (int)memoryStats.totalUsage);

	free(memory);
}