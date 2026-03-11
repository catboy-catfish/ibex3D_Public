#pragma once

#include "timerClass.h"

class appInterface;
class appRuntime
{
public:
	bool initialize(int wndWidth, int wndHeight, const char* wndTitle);
	void startRunning();
	void update();
	void forceClose();
	void cleanup();

	// ----------------------------------------------------------------------------------------------------

	void windowEvent_onResize();
	void windowEvent_onFocus();
	void windowEvent_onUnfocus();
	void windowEvent_onClose();

private:
	bool initWindow(int wndWidth, int wndHeight, const char* wndTitle);
	void updateWindow();
	void cleanupWindow();

	// ----------------------------------------------------------------------------------------------------

	bool initApplication(int wndWidth, int wndHeight);
	void cleanupApplication();

	// ----------------------------------------------------------------------------------------------------

	bool isSafeToStartRunning();

private:
	void* pWindowData = nullptr;
	appInterface* pAppInterface = nullptr;
	timerClass m_timer;

	// ----------------------------------------------------------------------------------------------------

	bool m_keepRunningFlag = true;
};