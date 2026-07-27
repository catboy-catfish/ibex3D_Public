/*
	DEPRECATION NOTICE

	This header and its functions are deprecated because I don't find myself using them very often, and instead I just use the regular new and delete operators.
	If you ever want to use this, go ahead. It's unlikely to be removed, but it's deprecated unless I revisit memory allocation tracking.
*/

#pragma once

#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <utility>

// ----------------------------------------------------------------------------------------------------

size_t totalRamUsage = 0;

// ----------------------------------------------------------------------------------------------------

template <typename t>
t* i3D_allocate()
{
	size_t memSize = sizeof(t);
	totalRamUsage += memSize;
	printf("Allocated %zu bytes of RAM (now using %zu bytes).\n", memSize, totalRamUsage);

	void* mem = malloc(memSize);
	return new(mem) t;
}

template <typename t, typename u>
t* i3D_allocate(u&& args)
{
	size_t memSize = sizeof(t);
	totalRamUsage += memSize;
	printf("Allocated %zu bytes of RAM (now using %zu bytes).\n", memSize, totalRamUsage);

	void* mem = malloc(memSize);
	return new(mem) t(std::forward<u>(args));
}

template <typename t>
void i3D_free(t* mem)
{
	size_t memSize = sizeof(t);
	totalRamUsage -= memSize;
	printf("Freed %zu bytes of RAM (now using %zu bytes).\n", memSize, totalRamUsage);

	mem->~t();
	free(mem);
}