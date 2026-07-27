#include <ibex3D/core/runtime_win32.h>
#include <ibex3D/core/entryPoint_win32.h>

// ----------------------------------------------------------------------------------------------------

int ibex3D_entryPoint() // Fuck off VCR003
{		
	auto pRuntime = new i3D_runtime_win32;

	if (pRuntime->initialize(1280, 720, "Hello, ibex3D!"))
	{
		pRuntime->startRunning();
	}

	pRuntime->cleanup();
	delete pRuntime;

	return 0;
}