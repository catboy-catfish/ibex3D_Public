#pragma once

#include <stdio.h>
#include <stdlib.h>

static size_t totalUsedBytes = 0;

void* operator new(size_t size)
{
	totalUsedBytes += size;
	printf("Allocated %zu bytes (now using %zu bytes).\n", size, totalUsedBytes);

	return malloc(size);
}

void operator delete(void* memory, size_t size)
{
	totalUsedBytes -= size;
	printf("Deallocated %zu bytes (now using %zu bytes).\n", size, totalUsedBytes);

	free(memory);
}