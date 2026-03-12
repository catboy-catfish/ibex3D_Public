#include <sampleApp/appInterface.h>

#include <ibex3D/core/appRuntime.h>
#include <ibexVulkan/vkRenderingContext.h>

#define MAX_MESH_ROTATION 6.28f

// ----------------------------------------------------------------------------------------------------
// - Main events --------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool appInterface::initialize(appRuntime* runtimePtr, void* pWindow)
{
	m_appRuntime = runtimePtr;
	
	m_renderingContext = new vkRenderingContext;
	if (!m_renderingContext->initialize("ibex3D Sample Application", pWindow))
	{
		return false;
	}

	return true;
}

void appInterface::update(float deltaTime)
{
	m_meshRotation += deltaTime;

	if (m_meshRotation >= MAX_MESH_ROTATION)
	{
		// printf("Mesh rotation has exceeded 2*PI, wrapping...\n");
		m_meshRotation -= MAX_MESH_ROTATION;
	}
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