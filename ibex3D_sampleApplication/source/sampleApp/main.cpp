#include <ibex3D/core/entryPoint.h>
#include <ibex3D/core/appRuntime.h>
#include <ibex3D/utility/logger.h>

int ibex3D_entryPoint()
{	
	logger::logInfo("Test message!", __FILE__, __LINE__);
	
	auto runtime = new appRuntime;

	if (runtime->initialize(1280, 720, "Hello, ibex3D!"))
	{
		runtime->run();
	}

	runtime->cleanup();
	delete runtime;
	runtime = nullptr;

	return 0;
}