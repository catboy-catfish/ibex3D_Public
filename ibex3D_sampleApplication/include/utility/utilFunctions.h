#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define IBEX3D_BASSERT(condition)	\
if (!condition)						\
{									\
	return false;					\
}

template<typename T>
static T custom_clamp(T input, T min, T max)
{
	if (input < min) return min;
	if (input > max) return max;

	return input;
}

static std::vector<char> readFile(const std::string& filepath)
{
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		std::cout << "ERROR - readFile(): Couldn't open file at path \"" << filepath << "\".\n";
		return std::vector<char>();
	}

	size_t fileSize = static_cast<size_t>(file.tellg());

	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}