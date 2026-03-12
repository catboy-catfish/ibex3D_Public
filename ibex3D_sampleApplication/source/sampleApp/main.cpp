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