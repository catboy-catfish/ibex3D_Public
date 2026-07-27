#pragma once

#include <vector>

// ----------------------------------------------------------------------------------------------------

class i3D_fileUtils
{
public:
	// File reading
	static std::vector<char> getFileContents(const char* filePath);
};