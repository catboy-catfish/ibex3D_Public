#include <ibex3D/utility/miscellaneous.h>

#include <fstream>
#include <stdio.h>

// ----------------------------------------------------------------------------------------------------

std::vector<char> i3D_utils::readFile(const char* filePath)
{
	// Pick up from where you left off at https://youtu.be/Qbs9v1W7St8?si=guDlWk9bBXZzHxOe&t=476
	
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		fprintf(stderr, "UTILITY ERROR: Couldn't open the file at path \"%s\". Have you ensured that the provided file path is correct?\n", filePath);
		return std::vector<char>();
	}

	size_t fileSize = static_cast<size_t>(file.tellg());

	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}