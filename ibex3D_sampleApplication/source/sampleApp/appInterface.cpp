#include <sampleApp/appInterface.h>

#include <ibex3D/core/appRuntime.h>
#include <ibex3D/vulkan/vkRenderingContext.h>

#define MAX_MESH_ROTATION 6.283f

// ----------------------------------------------------------------------------------------------------
// - Main events --------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool appInterface::initialize(appRuntime* runtimePtr, void* pWindow)
{
	m_appRuntime = runtimePtr;
	
	m_renderingContext = new vkRenderingContext;
	if (!m_renderingContext->initialize(pWindow))
	{
		return false;
	}

	return true;
}

void appInterface::update(float deltaTime)
{
	// Measure frames per second
	m_frameCounter++;
	m_timer += deltaTime;

	if (m_timer >= 1.0f)
	{
		printf("%zu frames have passed this second.\n", m_frameCounter);
		m_frameCounter = 0;
		m_timer -= 1.0f;
	}

	// Update mesh rotation
	m_meshRotation += m_meshRotationSpeed * deltaTime;

	if (m_meshRotation >= MAX_MESH_ROTATION)
		m_meshRotation -= MAX_MESH_ROTATION;
}

void appInterface::render(float deltaTime)
{	
	if (m_renderingContext != nullptr)
	{
		m_renderingContext->setMeshRotation(m_meshRotation);
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

// ----------------------------------------------------------------------------------------------------
// - Window events ------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

void appInterface::windowEvent_onResize(int wndWidth, int wndHeight)
{
	if (m_renderingContext != nullptr)
	{
		m_renderingContext->enableResizedFlag();
	}
}

void appInterface::windowEvent_onFocus()
{
}

void appInterface::windowEvent_onUnfocus()
{
}

void appInterface::windowEvent_onClose()
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