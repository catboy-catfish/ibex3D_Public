#pragma once

#include <vector>

// ----------------------------------------------------------------------------------------------------

#define IBEX3D_BASSERT(condition)	\
if (!condition)						\
{									\
	return false;					\
}

// ----------------------------------------------------------------------------------------------------

class ibex3D_utilFunctions
{
public:
	// File reading
	static std::vector<char> readFile(const char* filePath);
};