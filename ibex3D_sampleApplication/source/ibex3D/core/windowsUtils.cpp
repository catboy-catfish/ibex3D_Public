#include <ibex3D/core/windowsUtils.h>

#include <comdef.h>
#include <string>
#include <stdio.h>

// ----------------------------------------------------------------------------------------------------

bool i3D_windowsUtils::getWindowDimensions(HWND hWnd, LONG& width, LONG& height)
{
	if (hWnd == nullptr)
	{
		fprintf(stderr, "WIN32 ERROR: Argument \"HWND hWnd\" of getWindowDimensions() is nullptr.\n");
		return false;
	}

	RECT wndRect = {};

	if (GetClientRect(hWnd, &wndRect) == 0)
	{
		fprintf(stderr, "WIN32 ERROR: Couldn't get the window dimensions because GetClientRect() failed.\n");
		return false;
	}

	width = wndRect.right - wndRect.left;
	height = wndRect.bottom - wndRect.top;

	return true;
}

const char* i3D_windowsUtils::hresultToText(HRESULT hr)
{
	// FIX: Text is only readable in release mode, not debug mode.

	_com_error error(hr);
	const TCHAR* tcharErrMsg = error.ErrorMessage();

#ifdef UNICODE
	int bufferSize = WideCharToMultiByte
	(
		CP_UTF8,
		0,
		tcharErrMsg,
		-1,
		NULL,
		0,
		NULL,
		NULL
	);

	if (bufferSize == 0) return "";

	std::string narrowStr(bufferSize, 0);
	WideCharToMultiByte
	(
		CP_UTF8,
		0,
		tcharErrMsg,
		-1,
		&narrowStr[0],
		bufferSize,
		NULL,
		NULL
	);

	const char* charErrMsg = narrowStr.c_str();
	return charErrMsg;
#else
	return tcharErrMsg;
#endif
}