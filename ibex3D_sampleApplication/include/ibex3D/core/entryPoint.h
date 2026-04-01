#pragma once

#if defined(_CONSOLE)

#define ibex3D_entryPoint(args) main(args)

#else

#include <ibex3D/utility/windowsUtils.h>

#define ibex3D_entryPoint(args) WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)

#endif