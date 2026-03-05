#pragma once

#include <string>
#include <vector>

#define IBEX3D_BASSERT(condition)	\
if (!condition)						\
{									\
	return false;					\
}

template<typename T>
static T clamp(T input, T min, T max)
{
	if (input < min) return min;
	if (input > max) return max;

	return input;
}

class ibex3D_utilFunctions
{
public:
	// File reading
	static std::vector<char> readFile(const std::string& filePath);
};