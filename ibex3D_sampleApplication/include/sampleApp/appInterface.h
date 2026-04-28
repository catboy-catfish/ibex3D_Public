#pragma once

class appRuntime;
class vkRenderingContext;

class appInterface
{
public:
	
	/* Required setup functions */

	bool initialize(appRuntime* pRuntime, void* pWindow);
	void update(float deltaTime);
	void render(float deltaTime);
	void cleanup();

	void input_onKeyDownEvent(unsigned int key);
	void input_onKeyUpEvent(unsigned int key);
	bool input_isKeyDown(unsigned int key);

	void window_onResizeEvent(unsigned int wndWidth, unsigned int wndHeight);
	void window_onFocusEvent();
	void window_onUnfocusEvent();
	void window_onCloseRequestedEvent();

	bool isSafeToStartRunning();

	/* Examples of some functions you could create yourself for your own ibex3D project */

	void updateFpsCounter(float deltaTime);
	void updateMeshRotation(float deltaTime);

private:

	/* Required setup variables */
	
	bool m_keyStates[256] = { false };
	appRuntime* m_appRuntime = nullptr;
	vkRenderingContext* m_renderingContext = nullptr;

	/* Example variables - add or remove as needed */

	float m_meshRot = 0.0f;
	float m_meshRotVel = 0.0f;
	float m_meshRotSpd = 1.0f;

	float m_elapsedTime = 0.0f;
	size_t m_elapsedFrames = 0;
};