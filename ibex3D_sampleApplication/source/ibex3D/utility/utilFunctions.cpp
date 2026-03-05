#include <ibex3D/utility/utilFunctions.h>

#include <fstream>
#include <stdio.h>

std::vector<char> ibex3D_utilFunctions::readFile(const std::string& filePath)
{
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		printf("UTILITY ERROR - ibex3D_utilFunctions::readFile(): Couldn't open file at path \"%s\".\n", filePath.c_str());
		return std::vector<char>();
	}

	size_t fileSize = static_cast<size_t>(file.tellg());

	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}