#pragma once

#include "ibexWindows.h"

// ----------------------------------------------------------------------------------------------------

class i3D_windowsUtils
{
public:
	static bool getWindowDimensions(HWND hWnd, LONG& width, LONG& height);
	static const char* hresultToText(HRESULT hr);
};