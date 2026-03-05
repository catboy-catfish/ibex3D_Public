#pragma once

#include <ibex3D/core/timerClass.h>

class appInterface;
class appRuntime
{
public:
	bool initialize(int wndWidth, int wndHeight, const char* wndTitle);
	void startRunning();
	void update();
	void onWindowResize();
	void onWindowFocus();
	void onWindowUnfocus();
	void onWindowClose();
	void close();
	void cleanup();

private:
	bool initWindow(int wndWidth, int wndHeight, const char* wndTitle);
	void updateWindow();
	void cleanupWindow();

	bool initApplication(int wndWidth, int wndHeight);
	void cleanupApplication();

	bool isSafeToStartRunning();

private:
	void* pWindowData = nullptr;
	appInterface* pAppInterface = nullptr;
	timerClass m_timer;

	bool m_keepRunningFlag = true;

	int m_wndWidth = 0;
	int m_wndHeight = 0;
};