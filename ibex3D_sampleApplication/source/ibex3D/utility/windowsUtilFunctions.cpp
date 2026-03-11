#include <ibex3D/utility/windowsUtils.h>

#include <comdef.h>
#include <string>
#include <stdio.h>

bool win32Utils::getWindowDimensions(HWND hWnd, int& width, int& height)
{
	if (hWnd == nullptr)
	{
		printf("WIN32 ERROR - win32Utils::getWindowDimensions(): Argument \"HWND hWnd\" is nullptr.\n");
		return false;
	}

	RECT wndRect = {};

	if (GetClientRect(hWnd, &wndRect) == 0)
	{
		printf("WIN32 ERROR - win32Utils::getWindowDimensions(): Failed to get the client window rect.\n");
		return false;
	}

	width = static_cast<int>(wndRect.right - wndRect.left);
	height = static_cast<int>(wndRect.bottom - wndRect.top);

	return true;
}

const char* win32Utils::hresultToText(HRESULT hr)
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