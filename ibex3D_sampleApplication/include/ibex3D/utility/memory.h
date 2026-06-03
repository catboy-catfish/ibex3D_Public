#pragma once

#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <utility>

// ----------------------------------------------------------------------------------------------------

static size_t totalRamUsage = 0;

// ----------------------------------------------------------------------------------------------------

template <typename t>
t* ibex3D_allocate()
{
	size_t memSize = sizeof(t);
	totalRamUsage += memSize;
	printf("Allocated %zu bytes of RAM (now using %zu bytes).\n", memSize, totalRamUsage);

	void* mem = malloc(memSize);
	return new(mem) t;
}

template <typename t, typename u>
t* ibex3D_allocate(u&& args)
{
	size_t memSize = sizeof(t);
	totalRamUsage += memSize;
	printf("Allocated %zu bytes of RAM (now using %zu bytes).\n", memSize, totalRamUsage);

	void* mem = malloc(memSize);
	return new(mem) t(std::forward<u>(args));
}

template <typename t>
void ibex3D_free(t* mem)
{
	size_t memSize = sizeof(t);
	totalRamUsage -= memSize;
	printf("Freed %zu bytes of RAM (now using %zu bytes).\n", memSize, totalRamUsage);

	mem->~t();
	free(mem);
}