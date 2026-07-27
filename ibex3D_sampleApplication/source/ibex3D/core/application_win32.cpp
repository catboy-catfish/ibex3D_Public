#include <ibex3D/core/application_win32.h>
#include <ibex3D/core/runtime_win32.h>
#include <ibex3D/core/logger.h>

#include <ibex3D/vulkan/renderingContext.h>

// - https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes -----------------------

#define KEY_A 0x41
#define KEY_D 0x44
#define KEY_F 0x46

// ----------------------------------------------------------------------------------------------------

bool i3D_application_win32::initialize(i3D_runtime_win32* pRuntime, HWND hWnd)
{
	if (pRuntime == nullptr)
	{
		i3D_logErrorMessage("APPLICATION ERROR: Couldn't initialize the application because argument \"i3D_runtime_win32* pRuntime\" is nullptr.\n");
		return false;
	}
	
	m_runtimePtr = pRuntime;

	m_renderingContext = new i3D_vkRenderingContext;
	
	if (!m_renderingContext->initialize(hWnd))
	{
		i3D_logErrorMessage("APPLICATION ERROR: Couldn't initialize the application class because the Vulkan rendering context failed to initialize.\n");
		return false;
	}

	i3D_logInfoMessage("APPLICATION INFO: Successfully initialized the application class.\n");
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

		i3D_logInfoMessage("APPLICATION INFO: Cleaned up the Vulkan rendering context.\n");
	}

	m_runtimePtr = nullptr;

	i3D_logInfoMessage("APPLICATION INFO: Cleaned up the application class.\n");
}

void i3D_application_win32::onKeyDown(WPARAM key)
{
	m_keyStates[key] = true;
	i3D_logInfoMessage("APPLICATION INFO: Keyboard key (WPARAM: %X) pressed.\n", key);

	if (key == KEY_F)
	{
		m_runtimePtr->setWindowTitle("Respects");
	}
}

void i3D_application_win32::onKeyAutoRepeat(WPARAM key)
{
	i3D_logInfoMessage("APPLICATION INFO: Keyboard key (WPARAM: %X) auto-repeated.\n", key);
}

void i3D_application_win32::onKeyUp(WPARAM key)
{
	m_keyStates[key] = false;
	i3D_logInfoMessage("APPLICATION INFO: Keyboard key (WPARAM: %X) released.\n", key);

	if (key == KEY_F)
	{
		m_runtimePtr->setWindowTitle("Dangerfield");
	}
}

bool i3D_application_win32::isKeyDown(int key)
{
	return m_keyStates[key];
}

void i3D_application_win32::onWindowResize(WORD wndWidth, WORD wndHeight)
{
	i3D_logInfoMessage("APPLICATION INFO: Window resized to dimensions { %u, %u }.\n", wndWidth, wndHeight);
	
	if (m_renderingContext != nullptr)
	{
		m_renderingContext->refresh();
	}
}

void i3D_application_win32::onWindowFocus()
{
	i3D_logInfoMessage("APPLICATION INFO: Window gained user focus.\n");
}

void i3D_application_win32::onWindowUnfocus()
{
	i3D_logInfoMessage("APPLICATION INFO: Window lost user focus.\n");
}

void i3D_application_win32::onWindowCloseRequest()
{
	i3D_logInfoMessage("APPLICATION INFO: Window close button clicked.\n");
}

void i3D_application_win32::updateFpsCounter(float deltaTime)
{	
	m_elapsedFrames++;
	m_elapsedTime += deltaTime;

	if (m_elapsedTime >= 1.0f)
	{		
		// Mystery bug present since commit ac69e1c8: 
		// m_elapsedTime seems to rise more slowly while the window is minimized, but everything else I've checked seems to work fine
		
		i3D_logInfoMessage("APPLICATION INFO: %zu frames have passed this second.\n", m_elapsedFrames);
		
		m_elapsedFrames = 0;
		m_elapsedTime = 0.0f;
	}
}

void i3D_application_win32::updateMeshRotation(float deltaTime)
{
	m_meshRotVel = (isKeyDown(KEY_A) * -m_meshRotSpd) + (isKeyDown(KEY_D) * m_meshRotSpd);

	m_meshRot += m_meshRotVel * deltaTime;
}