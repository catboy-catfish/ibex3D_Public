#include <utility/windowsUtilFunctions.h>

#include <comdef.h>
#include <iostream>
#include <string>

bool win32_utils::getWindowDimensions(HWND hWnd, int& width, int& height)
{
	if (hWnd == nullptr)
	{
		std::cout << "WINDOWS UTIL ERROR - getWindowDimensions(): Argument \"HWND hWnd\" is nullptr.\n";
		return false;
	}

	RECT wndRect = {};

	if (GetClientRect(hWnd, &wndRect) == 0)
	{
		std::cout << "WINDOWS UTIL ERROR - getWindowDimensions(): Failed to get the client window rect.\n";
		return false;
	}

	width = static_cast<int>(wndRect.right - wndRect.left);
	height = static_cast<int>(wndRect.bottom - wndRect.top);

	return true;
}

const char* win32_utils::hresultToText(HRESULT hr)
{
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

	if (bufferSize == 0)
	{
		return "";
	}

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