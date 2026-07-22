#pragma once

#include <vector>

// ----------------------------------------------------------------------------------------------------

#define I3D_BASSERT(condition)		\
if (!condition)						\
{									\
	return false;					\
}

// ----------------------------------------------------------------------------------------------------

class i3D_utils
{
public:
	// File reading
	static std::vector<char> readFile(const char* filePath);
};