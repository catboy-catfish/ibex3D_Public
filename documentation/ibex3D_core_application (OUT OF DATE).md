# application - Basic overview

- Header file: `include/ibex3D/core/application.h`
- Source file: `source/ibex3D/core/application.cpp`

### Table of Contents

- [Description](#description)
- [Functions](#functions)
  - [Main functions](#main-functions)
  - [Input events and functions](#input-events-and-functions)
  - [Helper functions](#helper-functions)
  - [Custom function templates](#custom-function-templates)
- [Member variables](#member-variables)
  - [Main member variables](#main-member-variables)
  - [Custom member variable templates](#custom-member-variable-templates)
- [Examples](#examples)
- [To-do-list](#to-do-list)

### Description

`application` is the class responsible for housing and executing all of the application logic that occurs when the resources are initialized/cleaned up, and when the application is updated/rendered. This class is responsible for providing areas in the code where the most appropriate actions for the resources can be performed. The main functions, like `initialize()`, `update()`, `render()` and `cleanup()`, are automatically called by the `runtime` instance that owns it by default.

The flow of execution from the `main.cpp` file to this class is as follows: `int main()` -> `runtime` -> `application`

The sample application currently only features a simple mesh which can be rotated by using the A and D keys, and also counts the number of frames elapsed every second and prints the results to the console. You must modify the main functions to make it do what you want for the game you want to create.

`application` owns an instance of `vkRenderingContext`, allowing it to render graphics to the window using the Vulkan API, and also has a pointer to the `runtime` instance that owns it, allowing for back-and-forth communication between the two classes.

### Functions

##### Main functions

`bool initialize(runtime* pRuntime, void* pWindow)`
- Called by `runtime::initialize()` during application setup. This function should be used to allocate memory and initialize the game resources that need to be initialized on startup, such as the player/enemies, level data and items, and other state information.
- In the default sample application, it attempts to assign the memory address of `pRuntime` to `m_runtime` so that this `application` instance has a pointer to the `runtime`. If `pRuntime` is nullptr, initialization will fail and the function returns false (you can change this if you want, but it's not recommended). Then, it allocates the memory for `m_renderingContext` and attempts to initialize it using the typeless pointer `pWindow` (which points to the window memory and is internally used in `vkRenderingContext` for several areas in setup/rendering). The function returns true if initialization is successful, and false otherwise.

`void update(float deltaTime)`
- Called by `runtime::update()` for every time the game should be updated. This function should be used to update the state of the game resources every frame, like the player/enemies, level data and items.
- The `deltaTime` parameter, calculated and provided by the `runtime`, is the elapsed time (in seconds) between the start of the last frame and the current frame. This is very useful for updating game logic at a constant, predictable speed regardless of frame rate; for example, if you want some object to move at a constant speed, you should accumulate its position by its velocity multiplied by `deltaTime` (instead of just its velocity) so that the speed of the object isn't dependent on the framerate.

`void render(float deltaTime)`
- Called by `runtime::update()` every time the game should be render graphics to the window, after all the logic in `update()` has been executed for the current frame.
- The `deltaTime` parameter may be less useful here than in the `update()` function, but I included it in case it might be useful for updating temporal effects (if any are implemented).
- In the default sample application, every time this function is called, `m_renderingContext` sets its internal member variable `m_currentMeshRotation` (used for rendering the object) to the member variable`m_meshRot`, and then renders the image to the window using that variable.

`void cleanup()`
- Called by `runtime::cleanup()` after the game closes. This function should be used to free all allocated memory and perform other relevant cleanup tasks right before the entire application exits.
- In the default sample application, this function cleans up `m_renderingContext`, deallocates its memory, and then sets `m_runtime` to nullptr when it's no longer needed.

##### Input events and functions

`void input_onKeyDownEvent(unsigned int key)`
- Called by the `runtime` in the event where a keyboard key, with the code `key`, is pressed.

`void input_onKeyUpEvent(unsigned int key)`
- Called by the `runtime` in the event where a keyboard key, with the code `key`, is released.

`bool input_isKeyDown(unsigned int key)`
- Returns true if the keyboard key with the code `key` is detected to be pressed right now, and false otherwise.

`void window_onResizeEvent(unsigned int wndWidth, unsigned int wndHeight)`
- Called by the `runtime` in the event where the window is resized.
- The input parameters `wndWidth` and `wndHeight` represent the new width and height of the window after it is resized, respectively. When the window is minimized, they will both have a value of 0.

`void window_onFocusEvent()`
- Called by the `runtime` in the event where the window gains user focus.

`void window_onUnfocusEvent()`
- Called by the `runtime` in the event where the window loses user focus.

`void window_onCloseRequestedEvent()`
- Called by the `runtime` in the event where the user requests to close the application window (e.g. they click the close/X button on the top right/left corner of the window).
- This is not to be confused with the `cleanup()` function, which cleans up all resources when the application has finished running, although you could technically use this function to serve a similar purpose for some resources.

##### Helper functions

`bool isSafeToStartRunning()`
- Utility function called by `runtime::isSafeToStartRunning()` that checks whether or not the member variables `m_runtime` and `m_renderingContext` are properly initialized.

##### Custom function templates

```
DISCLAIMER: These are examples of functions you could implement yourself for your own ibex3D project. They are not called by the parent runtime, but you could call them in update(), render(), or any of the event functions, etc. as needed. Add/modify/remove them depending on what you need for your own project.
```

`void updateFpsCounter(float deltaTime)`
- Every time this function is called, the member variable `m_elapsedFrames` is incremented by 1 and `m_elapsedTime` is incremented by `deltaTime`. If `m_elapsedTime` is greater than or equal to 1, a message containing the value of `m_elapsedFrames` is printed to the console before both `m_elapsedFrames` and `m_elapsedTime` are reset to 0, restarting the timer and frame counter.

`void updateMeshRotation(float deltaTime)`
- Every time this function is called, if the key A is pressed, the member variable `m_meshRotVel` is set to -`m_meshRotSpd`, and if the key D is pressed, `m_meshRotVel` is set to `m_meshRotSpd`. In any case, in the end, `m_meshRot` is incremented by `m_meshRotVel` multiplied by `deltaTime`. This has the effect of making the mesh rotate clockwise along the Z axis when the key A is held down, and counter-clockwise when the key D is held down.

### Member variables

##### Main member variables

`bool m_keyStates[256]`
- An array of booleans tracking whether or not the keyboard key at a certain index is pressed (true) or released (false). This is automatically updated by the functions `input_onKeyDownEvent()` and `input_onKeyUpEvent()`, and is also used by the function `input_isKeyDown()`.

`runtime* m_runtime`
- A pointer to the `runtime` instance that owns this `application` instance. This can be used for back-and-forth communication between the two (instead of just one-way communication from the `runtime` to the `application`).
- This `application` instance does _not_ own the memory at the address stored in this member variable.

`vkRenderingContext* m_renderingContext`
- A pointer to the `vkRenderingContext` instance used for rendering. Unlike `m_runtime`, the memory pointed to by this variable _is_ owned and has its memory managed by the `application`.

##### Custom member variable templates

`float m_meshRot`
- The variable storing the mesh rotation for the `updateMeshRotation()` function. 
- The value of `m_meshRotVel` multiplied by `deltaTime` is added to this variable every frame.

`float m_meshRotVel`
- The variable storing the mesh rotation velocity for the `updateMeshRotation()` function.
- Whenever the A or D keys are held down, this variable is set to -`m_meshRotSpd` or `m_meshRotSpd` respectively.

`float m_meshRotSpd`
- The variable storing the mesh rotation speed for the `updateMeshRotation()` function.
- The value of this variable is fixed by default, but you can tweak it to taste or even change it at runtime by pressing a key if you want.

`float m_elapsedTime`
- The timer variable for the `updateFpsCounter()` function, which is incremented by `deltaTime` every frame.
- When the value reaches or exceeds 1, a message containing the value of `m_elapsedFrames` is printed to the console before this variable, along with `m_elapsedFrames`, is reset to 0, restarting the timer.

`size_t m_elapsedFrames`
- The number of elapsed frames used in the `updateFpsCounter()` function, which is incremented by 1 every frame.
- When the value of `m_elapsedTime` reaches or exceeds 1, a message containing the value of this variable is printed to the console before this variable, along with `m_elapsedTime`, is reset to 0, restarting the timer.

### Examples

How to initialize, cleanup and update/render using an `application` instance (as used in the file `source/ibex3D/core/runtime_win32.cpp`)
```cpp
// Initialization - called once in the initialization stage
bool runtime::initApplication(unsigned int wndWidth, unsigned int wndHeight)
{
	// Declaration in header: application* m_application = nullptr;
	m_application = new application;

	// m_windowData is a void*, so we cast it back to the platform-specific type windowData_t specified in this file
	auto wndData = static_cast<windowData_t*>(m_windowData);

	// In this source file, we take the HWND from wndData and cast it to a void*
	if (!m_application->initialize(this, static_cast<void*>(wndData->hWnd)))
	{
		logger::logError("runtime::initApplication(): The application failed to initialize.", __FILE__, __LINE__ - 2);
		return false;
	}

	return true;
}

// Cleanup - called once in the cleanup stage
void runtime::cleanupApplication()
{
	if (m_application != nullptr)
	{
		m_application->cleanup();
		delete m_application;
		m_application = nullptr;
	}
}

// Updating and rendering - called every time the window events are processed
void runtime::update()
{
	// Calculate the delta time, which is the time elapsed (in seconds) between the start of the previous frame and the start of the current one.
	auto endTime = std::chrono::high_resolution_clock::now();
	float deltaTime = std::chrono::duration<float>(endTime - m_startTime).count();
	m_startTime = endTime;

	// Update the application and tell it to render using delta time.
	if (m_application != nullptr)
	{
		m_application->update(deltaTime);
		m_application->render(deltaTime);
	}

	// ...
}
```

### To-do list

- `application` creates and owns an instance of `vkRenderingContext`, which is a Vulkan-specific type. If ibex3D is to support multiple graphics APIs in the future, there should be a better abstraction for this (for example, a wrapper class named `renderingFrontend` which hides the `vkRenderingContext`) that the `applpication`  can use instead of directly using `vkRenderingContext`.