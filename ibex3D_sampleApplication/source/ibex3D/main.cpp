#include <ibex3D/core/entryPoint.h>
#include <ibex3D/core/runtime.h>

int ibex3D_entryPoint()
{		
	auto pRuntime = new runtime;

	if (pRuntime->initialize(1280, 720, "Hello, ibex3D!"))
	{
		pRuntime->run();
	}

	pRuntime->cleanup();
	delete pRuntime;

	return 0;
}