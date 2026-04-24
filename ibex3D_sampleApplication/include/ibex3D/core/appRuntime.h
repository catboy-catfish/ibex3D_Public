#pragma once

#include <chrono>

class appInterface;
class appRuntime
{
public:
	bool initialize(unsigned int wndWidth, unsigned int wndHeight, const char* wndTitle);
	void run();
	void update();
	void forceClose();
	void cleanup();

	void window_onKeyDownEvent(unsigned int key);
	void window_onKeyUpEvent(unsigned int key);
	void window_onResizeEvent(unsigned int newWidth, unsigned int newHeight);
	void window_onFocusEvent();
	void window_onUnfocusEvent();
	void window_onCloseRequestedEvent();

private:
	bool initWindow(unsigned int wndWidth, unsigned int wndHeight, const char* wndTitle);
	void updateWindow();
	void cleanupWindow();

	bool initApplication(unsigned int wndWidth, unsigned int wndHeight);
	void cleanupApplication();

	bool isSafeToStartRunning();

private:
	void* m_windowData = nullptr;
	appInterface* m_appInterface = nullptr;

	std::chrono::high_resolution_clock::time_point m_startTime = std::chrono::high_resolution_clock::now();
	bool m_keepRunningFlag = true;
};