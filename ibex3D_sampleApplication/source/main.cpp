#include <utility/memoryUtils.h>
#include <core/appRuntime.h>

int main()
{	
	auto runtime = new appRuntime;

	if (runtime->initialize(1280, 720, "Hello, ibex3D!"))
	{
		runtime->startRunning();
	}

	runtime->cleanup();
	delete runtime;
	runtime = nullptr;
	
	// Warning: Big memory leak seems to occur here, even after everything has been cleaned up.

	// If you're using the Visual Studio debugger, close the main window to trigger the while loop, and then close the console window to close the application.
	// Pay attention to the memory usage in the debug output window. It stays at about 60 mb (on my machine) and doesn't go down, even after cleanup.

	system("pause");
	return 0;
}