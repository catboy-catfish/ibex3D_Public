#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// ----------------------------------------------------------------------------------------------------

class i3D_win32utils
{
public:
	static bool getWindowDimensions(HWND hWnd, int& width, int& height);
	static const char* hresultToText(HRESULT hr);
};