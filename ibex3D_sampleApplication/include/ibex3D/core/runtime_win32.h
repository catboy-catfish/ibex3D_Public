#pragma once

#include "ibexWindows.h"

#include <chrono>

class i3D_application_win32;

class i3D_runtime_win32
{
private:
	HWND m_hWnd = nullptr;

	i3D_application_win32* m_application = nullptr;

	std::chrono::high_resolution_clock::time_point m_startTime = std::chrono::high_resolution_clock::now();

	bool m_keepRunning = true;

private:
	bool initWindow(int width, int height, const char* title);
	void updateWindow();
	void cleanupWindow();

	bool initApplication();
	void updateApplication();
	void closeApplication();
	void cleanupApplication();

public:
	LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool initialize(int wndWidth, int wndHeight, const char* wndTitle);
	void startRunning();
	void cleanup();

	void setWindowTitle(const char* value);
	void setCursorVisibility(bool value);
};