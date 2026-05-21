#pragma once

#include <vector>

// - Macro definition ---------------------------------------------------------------------------------

#define IBEX3D_BASSERT(condition)	\
if (!condition)						\
{									\
	return false;					\
}

// - Class declaration --------------------------------------------------------------------------------

class ibex3D_utilFunctions
{
public:
	// File reading
	static std::vector<char> readFile(const char* filePath);
};