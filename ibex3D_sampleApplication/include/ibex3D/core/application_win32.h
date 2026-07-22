#pragma once

#include "win32.h"

// ----------------------------------------------------------------------------------------------------

class i3D_runtime_win32;
class i3D_vkRenderingContext;

// ----------------------------------------------------------------------------------------------------

class i3D_application_win32
{
public:
	bool initialize(i3D_runtime_win32* pRuntime, HWND hWnd);
	void update(float deltaTime);
	void render(float deltaTime);
	void cleanup();

	void onKeyDown(WPARAM key);
	void onKeyAutoRepeat(WPARAM key);
	void onKeyUp(WPARAM key);
	bool isKeyDown(int key);

	void onWindowResize(WORD wndWidth, WORD wndHeight);
	void onWindowFocus();
	void onWindowUnfocus();
	void onWindowCloseRequest();

	void updateFpsCounter(float deltaTime);
	void updateMeshRotation(float deltaTime);

private:
	bool m_keyStates[256] = { false };

	i3D_runtime_win32* m_runtime = nullptr;
	i3D_vkRenderingContext* m_renderingContext = nullptr;

	float m_meshRot = 0.0f;
	float m_meshRotVel = 0.0f;
	float m_meshRotSpd = 1.0f;
	float m_elapsedTime = 0.0f;
	size_t m_elapsedFrames = 0;
};