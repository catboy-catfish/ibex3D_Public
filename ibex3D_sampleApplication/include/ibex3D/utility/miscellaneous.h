#pragma once

#include <string>
#include <vector>

#define IBEX3D_BASSERT(condition)	\
if (!condition)						\
{									\
	return false;					\
}

class ibex3D_utilFunctions
{
public:
	// File reading
	static std::vector<char> readFile(const std::string& filePath);

	// Other
	static void fuck();
};