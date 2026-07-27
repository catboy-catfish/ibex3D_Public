#include <ibex3D/core/fileAccess.h>
#include <ibex3D/core/logger.h>

#include <fstream>

// ----------------------------------------------------------------------------------------------------

std::vector<char> i3D_fileUtils::getFileContents(const char* filePath)
{
	// Pick up from where you left off at https://youtu.be/Qbs9v1W7St8?si=guDlWk9bBXZzHxOe&t=476
	
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		i3D_logErrorMessage("FILE READING ERROR: Couldn't open file at path \"%s\".\n", filePath);
		return std::vector<char>();
	}

	size_t fileSize = static_cast<size_t>(file.tellg());

	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}