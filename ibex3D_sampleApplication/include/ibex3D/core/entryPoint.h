#pragma once

#if defined(_CONSOLE)

#define ibex3D_entryPoint() main()

#else

#include <ibex3D/core/win32.h>

#define ibex3D_entryPoint() WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)

#endif