#include <ibex3D/core/application_win32.h>
#include <ibex3D/core/runtime_win32.h>

#include <ibex3D/vulkan/renderingContext.h>

// - https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes -----------------------

#define KEY_A 0x41
#define KEY_D 0x44

// ----------------------------------------------------------------------------------------------------

bool i3D_application_win32::initialize(i3D_runtime_win32* pRuntime, HWND hWnd)
{
	if (pRuntime == nullptr)
	{
		fprintf(stderr, "APPLICATION ERROR: Couldn't initialize the application because argument \"i3D_runtime_win32* pRuntime\" is nullptr. You must pass a valid pointer to the i3D_runtime_win32 instance that owns this i3D_application_win32 instance so that it can send messages back to the i3D_runtime_win32 instance.\n");
		return false;
	}
	
	m_runtime = pRuntime;

	m_renderingContext = new i3D_vkRenderingContext;
	
	if (!m_renderingContext->initialize(hWnd))
	{
		fprintf(stderr, "APPLICATION ERROR: Failed to initialize the Vulkan rendering context.\n");
		return false;
	}

	return true;
}

void i3D_application_win32::update(float deltaTime)
{
	updateFpsCounter(deltaTime);
	updateMeshRotation(deltaTime);
}

void i3D_application_win32::render(float deltaTime)
{	
	if (m_renderingContext != nullptr)
	{
		m_renderingContext->drawFrame(m_meshRot);
	}
}

void i3D_application_win32::cleanup()
{
	if (m_renderingContext != nullptr)
	{
		m_renderingContext->cleanup();
		delete m_renderingContext;
		m_renderingContext = nullptr;
	}

	m_runtime = nullptr;
}

void i3D_application_win32::onKeyDown(WPARAM key)
{
	m_keyStates[key] = true;
}

void i3D_application_win32::onKeyAutoRepeat(WPARAM key)
{
}

void i3D_application_win32::onKeyUp(WPARAM key)
{
	m_keyStates[key] = false;
}

bool i3D_application_win32::isKeyDown(int key)
{
	return m_keyStates[key];
}

void i3D_application_win32::onWindowResize(WORD wndWidth, WORD wndHeight)
{
	if (m_renderingContext != nullptr)
	{
		m_renderingContext->refresh();
	}
}

void i3D_application_win32::onWindowFocus()
{
}

void i3D_application_win32::onWindowUnfocus()
{
}

void i3D_application_win32::onWindowCloseRequest()
{
}

void i3D_application_win32::updateFpsCounter(float deltaTime)
{	
	m_elapsedFrames++;
	m_elapsedTime += deltaTime;

	if (m_elapsedTime >= 1.0f)
	{		
		// Mystery bug present since commit ac69e1c8: 
		// This doesn't print while the window is minimized, but everything else I've checked seems to be working perfectly
		
		fprintf(stdout, "%zu frames have passed this second.\n", m_elapsedFrames);
		
		m_elapsedFrames = 0;
		m_elapsedTime = 0.0f;
	}

	static size_t temp_count = 0;
	temp_count++;
}

void i3D_application_win32::updateMeshRotation(float deltaTime)
{
	m_meshRotVel = (isKeyDown(KEY_A) * -m_meshRotSpd) + (isKeyDown(KEY_D) * m_meshRotSpd);

	m_meshRot += m_meshRotVel * deltaTime;
}