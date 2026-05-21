#include <ibex3D/core/application.h>
#include <ibex3D/core/runtime.h>

#include <ibex3D/vulkan/renderingContext.h>

#include <ibex3D/utility/logger.h>

// - Win32 virtual key codes --------------------------------------------------------------------------
// - https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes -----------------------

#define KEY_A 0x41
#define KEY_D 0x44

// - Main functions -----------------------------------------------------------------------------------

bool application::initialize(runtime* pRuntime, void* pWindow)
{
	/*
		Called by the parent runtime when the application is being initialized.
		Use this function to allocate memory, initialize variables, etc.
	*/

	if (pRuntime == nullptr)
	{
		logger::logError("application::initialize(): Couldn't initialize the application because argument \"runtime* pRuntime\" is nullptr. You must pass a pointer to the runtime instance that owns this application instance so that the application can communicate with it.", __FILE__, __LINE__ - 2);
		return false;
	}
	
	m_runtime = pRuntime;
	m_renderingContext = new vkRenderingContext;
	
	if (!m_renderingContext->initialize(pWindow))
	{
		logger::logError("application::initialize(): Couldn't initialize the application because the vkRenderingContext failed to initialize.", __FILE__, __LINE__ - 2);
		return false;
	}

	return true;
}

void application::update(float deltaTime)
{
	/*
		Called by the parent runtime every frame when the game should update its logic.
		Use this function to update all objects and variables, manage memory, etc.
	*/
	
	updateFpsCounter(deltaTime);
	updateMeshRotation(deltaTime);

	// FIX: This exception isn't caught by the try-catch block in runtime::run().
	// throw std::bad_cast(); // make sure to #include <exception>!
}

void application::render(float deltaTime)
{	
	/*
		Called by the parent runtime every frame after all logic in update() is completed.
		Use this function to render all objects, perform other relevant tasks, etc.
	*/
	
	if (m_renderingContext != nullptr)
	{
		m_renderingContext->setMeshRotation(m_meshRot);
		m_renderingContext->drawFrame();
	}
}

void application::cleanup()
{
	/*
		Called by the parent runtime when it has finished running and is cleaning up before exiting.
		Use this function to save any relevant data, free all allocated memory before exiting, etc.
	*/
	
	if (m_renderingContext != nullptr)
	{
		m_renderingContext->cleanup();
		delete m_renderingContext;
		m_renderingContext = nullptr;
	}

	m_runtime = nullptr;
}

void application::input_onKeyDownEvent(unsigned int key)
{
	/*
		Called by the parent runtime in the event where a key is pressed.
		Use this function to perform tasks depending on the key that was pressed, etc.
	*/

	m_keyStates[key] = true;
}

void application::input_onKeyUpEvent(unsigned int key)
{
	/*
		Called by the parent runtime in the event where a key is released.
		Use this function to perform tasks depending on the key that was released, etc.
	*/

	m_keyStates[key] = false;
}

bool application::input_isKeyDown(unsigned int key)
{
	/*
		Checks if a key is currently pressed or released right now.
		Returns true if the key is currently pressed, and false if it is currently released.
	*/
	
	return m_keyStates[key];
}

void application::window_onResizeEvent(unsigned int wndWidth, unsigned int wndHeight)
{
	/*
		Called by the parent runtime in the event where the window is resized.
		Use this function to refresh any rendering/UI data, update relevant variables, etc.

		The parameters wndWidth and wndHeight are the new width and height of the window after being resized, respectively.
		When the window is minimized, these parameters will both be 0, so be sure to handle that case as well if needed.
	*/
	
	if (m_renderingContext != nullptr)
	{
		m_renderingContext->enableResizedFlag();
	}
}

void application::window_onFocusEvent()
{
	/*
		Called by the parent runtime in the event where the window gains user focus.
	*/
}

void application::window_onUnfocusEvent()
{
	/*
		Called by the parent runtime in the event where the window loses user focus.
	*/
}

void application::window_onCloseRequestedEvent()
{
	/*
		Called by the parent runtime in the event where the window receives a close request from the user (e.g. they click the "X" button on the top right corner).
		Use this function to perform any relevant tasks before the cleanup stage occurs, etc.
		
		Not to be confused with the cleanup() function, which is called after this event is received when the application is actually being cleaned up before exiting.
		This function is only for performing tasks right when the close request is received, and not for when the application is actually being cleaned up before exiting.
	*/
}

bool application::isSafeToStartRunning()
{
	/*
		Utility function that is called by the parent runtime before starting the main loop to check if everything is properly initialized.
		This is called by runtime::isSafeToStartRunning(), which serves the same purpose for the runtime
		before starting the main loop to check if everything is properly initialized.
	*/
	
	if (m_runtime == nullptr)
	{
		return false;
	}

	if (m_renderingContext == nullptr)
	{
		return false;
	}
	
	return true;
}

// - Custom function examples -------------------------------------------------------------------------

void application::updateFpsCounter(float deltaTime)
{
	/*
		An example of a function you could create yourself for your own ibex3D project.
		This function is not called by the parent runtime, but you could call it in update(), render(), or any of the event functions, etc. as needed.
	*/
	
	m_elapsedFrames++;
	m_elapsedTime += deltaTime;

	if (m_elapsedTime >= 1.0f)
	{		
		printf("%zu frames have passed this second.\n", m_elapsedFrames);
		m_elapsedFrames = 0;
		m_elapsedTime = 0.0f;
	}
}

void application::updateMeshRotation(float deltaTime)
{
	if (input_isKeyDown(KEY_A))
		m_meshRotVel = -m_meshRotSpd;
	else
		m_meshRotVel = 0.0f;

	if (input_isKeyDown(KEY_D))
		m_meshRotVel += m_meshRotSpd;

	m_meshRot += m_meshRotVel * deltaTime;
}