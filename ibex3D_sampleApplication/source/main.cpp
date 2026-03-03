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
	
	// Warning: Big memory leak occurs here, even after everything has been cleaned up.
	// If you're using the Visual Studio debugger, try inserting "while (true) {}" to test it out at this point, 
	// and then exit the application from the console window.

	return 0;
}