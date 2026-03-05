#pragma once

class appRuntime;
class vk_renderingContext;

class appInterface
{
public:
	bool initialize(appRuntime* runtimePtr, void* pWindow);
	void update(float deltaTime);
	void render();
	void cleanup();

	void windowEvent_onResize(int wndWidth, int wndHeight);
	void windowEvent_onFocus();
	void windowEvent_onUnfocus();
	void windowEvent_onClose();

	bool isSafeToStartRunning();

private:
	appRuntime* m_appRuntime = nullptr;
	vk_renderingContext* m_renderingContext = nullptr;
};