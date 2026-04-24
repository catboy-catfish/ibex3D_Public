# appRuntime - Basic overview

* Header file: `include/ibex3D/core/appRuntime.h`
* Source file (Win32): `source/ibex3D/core/appRuntime\_win32.cpp`

### Table of Contents

- [Description](#description)
- [Functions](#functions)
	- [Public functions](#public-functions)
	- [Window procedure functions](#window-procedure-functions)
	- [Private functions](#private-functions)
- [Member variables](#member-variables)
- [Examples](#examples)

### Description

The `appRuntime` class represents the outermost execution layer of the application developed with ibex3D. This class is responsible for window creation and message handling, input processing and delta time calculation, and it also owns an instance of `appInterface`, being responsible for calling its main functions.

### Functions

##### Public functions

`bool initialize(int wndWidth, int wndHeight, const char* wndTitle)`
- Attempts to initialize the window and `appInterface` instance, taking in the target window width, height and title. This is technically just a helper function which calls `initWindow()` and `initApplication()` in the correct order.
- Returns true if both are successful and false otherwise.

`void startRunning()`
- Checks if it is safe to begin the application processing loop.
- If it is safe to do so, begins the processing loop. If not, immediately exits without doing anything.
- This function continues executing and blocks further execution until the application runtime ends.

`void forceClose()`
- Forcibly ends the processing loop and destroys the window, immediately closing the application.
- This is not to be confused with `cleanup()`, which frees all memory allocated by the window and appInterface after the application runtime ends.

`void cleanup()`
- Frees all memory allocated by the window and appInterface.
- This function is meant to be called after the application runtime ends. Please see the example below for an example of how this should be done.

##### Window procedure functions

> DISCLAIMER: These functions are meant to be called by the window procedure or the `appRuntime` itself, not the user. The reason why they are public is because it would otherwise be difficult for the window procedure to access them in a platform-agnostic way. I highly recommend you don't touch these functions or their call sites unless necessary.

`void update()`
- Calculates the delta time, which is the time elapsed between the start and end of the previous frame, and uses it to call the `appInterface`'s `update()` and `render()` functions.
- If the escape key is detected to be pressed, it immediately ends the process/update loop and exits the application. Feel free to remove this piece of code if you like (I know that this is an exception to the disclaimer above, but still).

`void window_onKeyDownEvent(unsigned int key)`
- Sends an event to the `appInterface` signaling that a specific `key` was just pressed.

`void window_onKeyUpEvent(unsigned int key)`
- Sends an event to the `appInterface` signaling that a specific `key` was just released.

`void window_onResizeEvent()`
- Sends an event to the `appInterface` signaling that the window was just resized, with the dimensions of the window after resizing passed in as arguments.

`void window_onFocusEvent()`
- Sends an event to the `appInterface` signaling that the window has just gained user focus.

`void window_onUnfocusEvent()`
- Sends an event to the `appInterface` signaling that the window has just lost user focus.

`void window_onCloseRequestedEvent()`
- Sends an event to the `appInterface` signaling that the user has just clicked the close button on the window.
- This is not to be confused with the "cleanup" functions, which are meant to free the allocated memory after the runtime ends, although the onCloseRequested event can also be used to free memory.

##### Private functions

`bool initWindow(int wndWidth, int wndHeight, const char* wndTitle)`
- Attempts to create the window used for displaying the application's graphics and shows it to the screen.
- The parameters `wndWidth` and `wndHeight` specify the window's dimensions upon creation, and `wndTitle` specifies the text displayed on the title bar.
- Returns true if all stages of the window initialization were successful, and false otherwise.

`void updateWindow()`
- Handles the window's internal message processing and event handling.
- This is not to be confused with `update()`, which calculates the delta time and updates the `appInterface` using it.

`void cleanupWindow()`
- Frees all memory allocated by the window.

`bool initApplication(int wndWidth, int wndHeight)`
- Attempts to create and initialize the `appInterface` instance.
- Returns true if successful and false otherwise.

`void cleanupApplication()`
- Calls the appInterface's cleanup() function, and then deallocates the instance memory.

`bool isSafeToStartRunning()`
- This is a helper function which decides if the application can safely start running by checking if all necessary member variables are initialized (not `nullptr`).
- Returns true if the application can safely start running, and false otherwise.

### Member variables

`void* m_windowData`
- The memory address to the window data struct instance, which contains platform-specific data like `HINSTANCE` and `HWND` on Win32.
- The reason why this is a `void*` is to make it easier to use a different window data struct depending on the platform, without exposing platform-specific information to the scope of the header file or wherever it's included.
- Different source files for `appRuntime` are for different platforms, and thus contain information specific to those different platforms.

`appInterface* m_appInterface`
- A pointer to the `appInterface` instance, which is allocated on the heap.

`std::chrono::high_resolution_clock::time_point m_startTime`
- The variable recording the exact time when the current loop of the application processing loop begins.
- This is used for the delta time calculation in the `update()` function.

`bool m_keepRunningFlag`
- The variable which controls whether the application processing loop should continue or stop immediately.
- This is set to true when the application should keep running, and false when it should quit.

### Examples

How to initialize, run and cleanup an appRuntime instance (as used in the entry point file, `source/sampleApp/main.cpp`)
```cpp
#include <ibex3D/core/entryPoint.h>
#include <ibex3D/core/appRuntime.h>

int ibex3D_entryPoint()
{
	auto pRuntime = new appRuntime;
	
	if (pRuntime->initialize(1280, 720, "Hello, ibex3D!")
	{
		pRuntime->startRunning();
	}
	
	pRuntime->cleanup();
	delete pRuntime;
	pRuntime = nullptr;
	
	return 0;
}
```