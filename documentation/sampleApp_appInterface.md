# appInterface - Basic overview

- Header file: `include/sampleApp/appInterface.h`
- Source file: `source/sampleApp/appInterface.cpp`

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

`appInterface` is the class responsible for housing and executing all of the logic that occurs when the game resources are initialized/cleaned up, and when the game is updated/rendered. This class is responsible for providing areas in the code where the most appropriate actions for the resources can be performed. The main functions, like `initialize()`, `update()`, `render()` and `cleanup()`, are automatically called by the `appRuntime` instance that owns it by default.

The flow of execution from the `main.cpp` file to this class is as follows: `int main()` -> `appRuntime` -> `appInterface`

The sample application currently only features a simple mesh which can be rotated by using the A and D keys, and also counts the number of frames elapsed every second and prints the results to the console. You must modify the main functions to make it do what you want for the game you want to create.

`appInterface` owns an instance of `vkRenderingContext`, allowing it to render graphics to the window using the Vulkan API, and also has a pointer to the `appRuntime` instance that owns it, allowing for back-and-forth communication between the two classes.

### Functions

##### Main functions

`bool initialize(appRuntime* pRuntime, void* pWindow)`
- Called by `appRuntime::initialize()` during application setup. This function should be used to allocate memory and initialize the game resources that need to be initialized on startup, such as the player/enemies, level data and items, and other state information.
- In the default sample application, it attempts to assign the memory address of `pRuntime` to `m_appRuntime` so that this appInterface instance has a pointer to the `appRuntime`. If `pRuntime` is nullptr, initialization will fail and the function returns false (you can change this if you want, but it's not recommended). Then, it allocates the memory for `m_renderingContext` and attempts to initialize it using the typeless pointer `pWindow` (which points to the window memory and is internally used in `vkRenderingContext` for several areas in setup/rendering). The function returns true if initialization is successful, and false otherwise.

`void update(float deltaTime)`
- Called by `appRuntime::update()` for every time the game should be updated. This function should be used to update the state of the game resources every frame, like the player/enemies, level data and items.
- The `deltaTime` parameter, calculated and provided by the `appRuntime`, is the elapsed time (in seconds) between the start of the last frame and the start of the current frame. This is very useful for updating game logic at a constant, predictable speed regardless of frame rate; for example, if you want some object to move at a constant speed, you should accumulate its position by its velocity multiplied by `deltaTime` (instead of just its velocity) so that the speed of the object isn't dependent on the framerate.

`void render(float deltaTime)`
- Called by `appRuntime::update()` every time the game should be render graphics to the window, after all the logic in `update()` has been executed for the current frame.
- The `deltaTime` parameter may be less useful here than in the `update()` function, but I included it in case it might be useful for updating temporal effects (if any are implemented).
- In the default sample application, every time this function is called, `m_renderingContext` sets its internal member variable `m_currentMeshRotation` (used for rendering the object) to the member variable`m_meshRot`, and then renders the image to the window using that variable.

`void cleanup()`
- Called by `appRuntime::cleanup()` after the game closes. This function should be used to free all allocated memory and perform other relevant cleanup tasks right before the entire application exits.
- In the default sample application, this function cleans up `m_renderingContext`, deallocates its memory, and then sets `m_appRuntime` to nullptr when it's no longer needed.

##### Input events and functions

`void input_onKeyDownEvent(unsigned int key)`
- Called by the `appRuntime` in the event where a keyboard key, with the code `key`, is pressed.

`void input_onKeyUpEvent(unsigned int key)`
- Called by the `appRuntime` in the event where a keyboard key, with the code `key`, is released.

`bool input_isKeyDown(unsigned int key)`
- Returns true if the keyboard key with the code `key` is detected to be pressed right now, and false otherwise.

`void window_onResizeEvent(unsigned int wndWidth, unsigned int wndHeight)`
- Called by the `appRuntime` in the event where the window is resized.
- The input parameters `wndWidth` and `wndHeight` represent the new width and height of the window after it is resized, respectively. When the window is minimized, they will both have a value of 0.

`void window_onFocusEvent()`
- Called by the `appRuntime` in the event where the window gains user focus.

`void window_onUnfocusEvent()`
- Called by the `appRuntime` in the event where the window loses user focus.

`void window_onCloseRequestedEvent()`
- Called by the `appRuntime` in the event where the user requests to close the application window (e.g. they click the close/X button on the top right/left corner of the window).
- This is not to be confused with the `cleanup()` function, which cleans up all resources when the application has finished running, although you could technically use this function to serve a similar purpose for some resources.

##### Helper functions

`bool isSafeToStartRunning()`
- Utility function called by `appRuntime::isSafeToStartRunning()` that checks whether or not the member variables `m_appRuntime` and `m_renderingContext` are properly initialized.

##### Custom function templates

```
DISCLAIMER: These are examples of functions you could implement yourself for your own ibex3D project. They are not called by the parent appRuntime, but you could call them in update(), render(), or any of the event functions, etc. as needed. Add/modify/remove them depending on what you need for your own project.
```

`void updateFpsCounter(float deltaTime)`
- Every time this function is called, the member variable `m_elapsedFrames` is incremented by 1 and `m_elapsedTime` is incremented by `deltaTime`. If `m_elapsedTime` is greater than or equal to 1, a message containing the value of `m_elapsedFrames` is printed to the console before both `m_elapsedFrames` and `m_elapsedTime` are reset to 0, restarting the timer and frame counter.

`void updateMeshRotation(float deltaTime)`
- Every time this function is called, if the key A is pressed, the member variable `m_meshRotVel` is set to -`m_meshRotSpd`, and if the key D is pressed, `m_meshRotVel` is set to `m_meshRotSpd`. In any case, in the end, `m_meshRot` is incremented by `m_meshRotVel` multiplied by `deltaTime`. This has the effect of making the mesh rotate clockwise along the Z axis when the key A is held down, and counter-clockwise when the key D is held down.

### Member variables

##### Main member variables

`bool m_keyStates[256]`
- An array of booleans tracking whether or not the keyboard key at a certain index is pressed (true) or released (false). This is automatically updated by the functions `input_onKeyDownEvent()` and `input_onKeyUpEvent()`, and is also used by the function `input_isKeyDown()`.

`appRuntime* m_appRuntime`
- A pointer to the `appRuntime` instance that owns this `appInterface` instance. This can be used for back-and-forth communication between the two (instead of just one-way communication from the `appRuntime` to the `appInterface`).
- This `appInterface` instance does not own the memory at the address stored in this member variable.

`vkRenderingContext* m_renderingContext`
- A pointer to the `vkRenderingContext` instance used for rendering. Unlike `m_appRuntime`, the memory pointed to by this variable _is_ owned by the `appInterface` and has its memory managed by the `appInterface`.

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

How to initialize, cleanup and update/render using an appInterface instance (as used in the file `source/ibex3D/core/appRuntime_win32.cpp`)
```cpp
// Initialization - called once in the initialization stage
bool appRuntime::initApplication(unsigned int wndWidth, unsigned int wndHeight)
{
    // Declaration in header: appInterface* m_appInterface = nullptr;
    m_appInterface = new appInterface;

    // m_windowData is a void*, so we cast it back to the platform-specific type windowData_t specified in this file
    auto wndData = static_cast<windowData_t*>(m_windowData);

    // In this source file, we take the HWND from wndData and cast it to a void*
    if (!m_appInterface->initialize(this, static_cast<void*>(wndData->hWnd)))
    {
        printf("RUNTIME ERROR - appRuntime::initApplication(): appInterface::initialize() failed.\n");
        return false;
    }

    return true;
}

// Cleanup - called once in the cleanup stage
void appRuntime::cleanupApplication()
{
    if (m_appInterface != nullptr)
    {
        m_appInterface->cleanup();
        delete m_appInterface;
        m_appInterface = nullptr;
    }
}

// Updating and rendering - called every time the window events are processed
void appRuntime::update()
{
    // Calculate deltaTime, the time (in seconds) elapsed between the start of the previous frame and this one
    auto endTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(endTime - m_startTime).count();
    m_startTime = endTime;

    // Update the appInterface and tell it to render using deltaTime
    if (m_appInterface != nullptr)
    {
        m_appInterface->update(deltaTime);
        m_appInterface->render(deltaTime);
    }

    // ...
}
```

### To-do list

- `appInterface` creates and owns an instance of `vkRenderingContext`, which is a Vulkan-specific type. If ibex3D is to support multiple graphics APIs in the future, there should be a better abstraction for this (for example, a wrapper class named `renderingFrontend` which hides the `vkRenderingContext`) that the `appInterface`  can use instead of directly using `vkRenderingContext`.