/*
	TODO:
	Pick up from where you left off at the creation of the staging buffer in the host memory.
	
	https://vulkan-tutorial.com/Compute_Shader
*/

// #include <ibex3D/utility/memoryUtils.h>
#include <ibex3D/core/appRuntime.h>

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

	return 0;
}