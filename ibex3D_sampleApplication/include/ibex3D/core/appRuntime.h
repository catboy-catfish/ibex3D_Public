#pragma once

#include <chrono>

class appInterface;
class appRuntime
{
public:
	bool initialize(int wndWidth, int wndHeight, const char* wndTitle);
	void startRunning();
	void update();
	void forceClose();
	void cleanup();

	void windowEvent_onResize();
	void windowEvent_onFocus();
	void windowEvent_onUnfocus();
	void windowEvent_onClose();

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
	
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();

	bool m_keepRunningFlag = true;
};