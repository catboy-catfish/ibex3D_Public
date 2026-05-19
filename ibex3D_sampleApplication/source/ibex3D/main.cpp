#include <ibex3D/core/entryPoint.h>
#include <ibex3D/core/runtime.h>
#include <ibex3D/utility/logger.h>

int ibex3D_entryPoint()
{		
	auto rt = new runtime;

	if (rt->initialize(1280, 720, "Hello, ibex3D!"))
	{
		rt->run();
	}

	rt->cleanup();
	delete rt;

	return 0;
}