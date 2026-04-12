#include <sampleApp/appInterface.h>

#include <ibex3D/core/appRuntime.h>
#include <ibex3D/vulkan/renderingContext.h>

/*
	Win32 virtual key codes.
	https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
*/

#define KEY_A 0x41
#define KEY_D 0x44

// ----------------------------------------------------------------------------------------------------
// - Main events --------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool appInterface::initialize(appRuntime* pRuntime, void* pWindow)
{
	if (pRuntime != nullptr)
	{
		m_appRuntime = pRuntime;
	}
	else
	{
		printf("APPLICATION ERROR - appInterface::initialize(): Argument \"appRuntime* pRuntime\" is nullptr. You must pass a valid appRuntime pointer for the appInterface to refer to.\n");
		return false;
	}
	
	m_renderingContext = new vkRenderingContext;
	
	if (!m_renderingContext->initialize(pWindow))
	{
		return false;
	}

	return true;
}

void appInterface::update(float deltaTime)
{
	m_elapsedFrames++;
	m_elapsedTime += deltaTime;

	if (m_elapsedTime >= 1.0f)
	{
		printf("%zu frames have passed this second.\n", m_elapsedFrames);
		m_elapsedFrames = 0;
		m_elapsedTime -= 1.0f;
	}

	// ----------------------------------------------------------------------------------------------------

	m_meshRotVel = (input_isKeyDown(KEY_A) ? -m_meshRotSpd : 0.0f);
	if (input_isKeyDown(KEY_D)) m_meshRotVel += m_meshRotSpd;

	m_meshRot += m_meshRotVel * deltaTime;

	if (m_meshRot >= 6.283f)
		m_meshRot -= 6.283f;
}

void appInterface::render(float deltaTime)
{	
	if (m_renderingContext != nullptr)
	{
		m_renderingContext->setMeshRotation(m_meshRot);
		m_renderingContext->drawFrame();
	}
}

void appInterface::cleanup()
{
	if (m_renderingContext != nullptr)
	{
		m_renderingContext->cleanup();
		delete m_renderingContext;
		m_renderingContext = nullptr;
	}

	m_appRuntime = nullptr;
}

void appInterface::input_onKeyDownEvent(unsigned int key)
{
	m_keyStates[key] = true;
}

void appInterface::input_onKeyUpEvent(unsigned int key)
{
	m_keyStates[key] = false;
}

bool appInterface::input_isKeyDown(unsigned int key)
{
	return m_keyStates[key];
}

// ----------------------------------------------------------------------------------------------------
// - Window events ------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

void appInterface::window_onResizeEvent(int wndWidth, int wndHeight)
{
	if (m_renderingContext != nullptr)
	{
		m_renderingContext->enableResizedFlag();
	}
}

void appInterface::window_onFocusEvent()
{
}

void appInterface::window_onUnfocusEvent()
{
}

void appInterface::window_onCloseRequestedEvent()
{
}

// ----------------------------------------------------------------------------------------------------
// - Helper functions ---------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool appInterface::isSafeToStartRunning()
{
	if (m_appRuntime == nullptr)
	{
		return false;
	}

	if (m_renderingContext == nullptr)
	{
		return false;
	}
	
	return true;
}