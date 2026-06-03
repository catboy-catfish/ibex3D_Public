#pragma once

// ----------------------------------------------------------------------------------------------------

class runtime;
class vkRenderingContext;

// ----------------------------------------------------------------------------------------------------

class application
{
public:
	bool initialize(runtime* pRuntime, void* pWindow);
	void update(float deltaTime);
	void render(float deltaTime);
	void cleanup();

	void input_onKeyDownEvent(unsigned int key);
	void input_onKeyRepeatEvent(unsigned int key);
	void input_onKeyUpEvent(unsigned int key);
	bool input_isKeyDown(unsigned int key);

	void window_onResizeEvent(unsigned int wndWidth, unsigned int wndHeight);
	void window_onFocusEvent();
	void window_onUnfocusEvent();
	void window_onCloseRequestedEvent();

	bool isSafeToStartRunning();

	// Example functions below - add or remove as needed

	void updateFpsCounter(float deltaTime);
	void updateMeshRotation(float deltaTime);

private:
	bool m_keyStates[256] = { false };

	runtime* m_runtime = nullptr;
	vkRenderingContext* m_renderingContext = nullptr;

	// Example variables below - add or remove as needed

	float m_meshRot = 0.0f;
	float m_meshRotVel = 0.0f;
	float m_meshRotSpd = 1.0f;
	float m_elapsedTime = 0.0f;
	size_t m_elapsedFrames = 0;
};