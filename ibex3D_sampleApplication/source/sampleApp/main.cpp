#include <ibex3D/utility/memoryUtils.h>
#include <ibex3D/core/appRuntime.h>

/*
	https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
	Pick up from where you left off at "Filling the vertex buffer".
*/

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