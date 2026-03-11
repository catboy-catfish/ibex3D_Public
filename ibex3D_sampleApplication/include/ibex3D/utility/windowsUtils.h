#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class win32Utils
{
public:
	static bool getWindowDimensions(HWND hWnd, int& width, int& height);
	static const char* hresultToText(HRESULT hr);
};