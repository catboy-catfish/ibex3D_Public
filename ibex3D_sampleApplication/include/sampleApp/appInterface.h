#pragma once

class appRuntime;
class vkRenderingContext;

class appInterface
{
public:
	bool initialize(appRuntime* pRuntime, void* pWindow);
	void update(float deltaTime);
	void render(float deltaTime);
	void cleanup();

	void input_onKeyDownEvent(unsigned int key);
	void input_onKeyUpEvent(unsigned int key);
	bool input_isKeyDown(unsigned int key);

	void window_onResizeEvent(int wndWidth, int wndHeight);
	void window_onFocusEvent();
	void window_onUnfocusEvent();
	void window_onCloseRequestedEvent();

	bool isSafeToStartRunning();

private:
	bool m_keyStates[256] = { false };

	appRuntime* m_appRuntime = nullptr;
	vkRenderingContext* m_renderingContext = nullptr;

	float m_elapsedTime = 0.0f;
	size_t m_elapsedFrames = 0;

	float m_meshRot = 0.0f;
	float m_meshRotVel = 0.0f;
	float m_meshRotSpd = 1.0f;
};