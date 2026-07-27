#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// ----------------------------------------------------------------------------------------------------

class i3D_windowsUtils
{
public:
	static bool getWindowDimensions(HWND hWnd, LONG& width, LONG& height);
	static const char* hresultToText(HRESULT hr);
};