# START HERE

### Table of Contents

- [To-Do List](#todo-list)
- [The Entry Point File](#the-entry-point-file)
	- [The Other Entry Point File](#the-other-entry-point-file)
- [Code Conventions](#code-conventions)
- [Preprocessor Definitions](#preprocessor-definitions)

### To-Do List

This is a to-do list for Sam and other developers (if any) to refer to.

Documentation-specific:
- Finalize documentation
- Try to keep up to date if you notice any discrepancies between this file and the source code/APIs!

General/Miscellaneous:
- Finalize the vec4 documentation
- Remove the `getSize()` functions from the vec2, vec3 and vec4 structs. Actually, I'll do this one myself, but not today because it's the end of the day.<br><br>These are absolutely worthless; they have no performance or convenience benefit over simply saying `sizeof(vec2)`, `sizeof(vec3)` or `sizeof(vec4)`, and to my knowledge, are also never used anywhere in ibex3D. I don't know why I implemented these in the first place since they already use `sizeof(float)`, but I just learned something: `sizeof(type)` is replaced with the size of `type` at compile time, which means it has no runtime overhead.
- Figure out how to render multiple objects in one scene, preferrably with different shaders for each
- Focus on pimping the engine code and architecture until you have the balls to make actual progress
- Investigate GLFW as a more stable, cross-platform replacement for the manual Win32 windowing code
- Switch from Visual Studio to something like CMake to become more platform and compiler-independent
- Use meta build systems to exclude platform-specific source code files depending on chosen build platform

C/++ Specific:
- Investigate and implement custom memory allocation functions instead of overriding the new and delete operators
- Look into custom memory allocation/allocators and data alignment

Vulkan-Specific:
- Find a way to compile GLSL code to SPIR-V bytecode at runtime using something like glslang or shaderc (may need to be compiled with static runtime library?)
- Find a way to allocate the vertex, index and uniform buffer memory with one allocation
- Fix the validation layer error that occurs during render pass creation whenever MSAA is disabled (VUID-VkSubpassDescription-pResolveAttachments-00848)
- Investigate push constants as a replacement for uniform buffers

Model importing:
- The .obj model file format is being ignored by the .gitignore, and I find the vertex/index loading code in vkMeshClass to be hideous since .obj files don't seem to support modern indexing by default. Transition to a better model file format for this like glTF or FBX instead.
- Better yet, start using a custom intermediate format for assets that is quickly loadable, efficient and customizable - [vkguide.dev has a tutorial for this](https://vkguide.dev/docs/extra-chapter/asset_system/).

Math vector types:
- Figure out how to implement SIMD into the vec4 +, -, * and / operators without it being *slower* than using SISD
- Figure out how to implement SIMD into the vec2 and vec3 types as well

### The Entry Point File

This section may not be necessary if you're experienced enough to find the entry point file yourself, but I'm personally not as experienced and have had issues with that in the past, so I'm just going to put this here in case it's helpful for anybody else with the same issue as me.

The entry point file for the ibex3D sample application is located at `ibex3D_sampleApplication/source/sampleApp/main.cpp`, where the `ibex3D_sampleApplication` folder is located along this file's parent folder and the `libraries` folder. The source code of this file looks something like this:

```cpp
#include <ibex3D/core/entryPoint.h>
#include <ibex3D/core/appRuntime.h>

int ibex3D_entryPoint()
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
```

The symbol `ibex3D_entryPoint` is a preprocessor macro in `ibex3D_sampleApplication/include/ibex3D/core/entryPoint.h` which simply switches between `main()` (which has an extra console window for debugging) and `WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)` (with no console window) depending on whether or not `_CONSOLE` is specified, which is itself specified in Debug mode and not in Release mode. The source code of that file looks something like this:

```cpp
#pragma once

#if defined(_CONSOLE)

#define ibex3D_entryPoint() main()

#else

#include <ibex3D/core/win32.h>
#define ibex3D_entryPoint() WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)

#endif
```

`<ibex3D/core/win32>` is a platform-specific utility header that includes `<Windows.h>` in order to access the Win32 API, a few preprocessor definitions like `WIN32_LEAN_AND_MEAN` and `NOMINMAX`, as well as `win32Utils`, a class with a few helper functions. The source code won't be pasted here in order to keep the file size down, but you can find it in the header file `include/ibex3D/core/win32.h`.

##### The Other Entry Point File

In addition to the main entry point file, `source/sampleApp/main.cpp`, the project also has another entry point file, `source/sampleApp/testbed_0.cpp`. This file is used for testing functions, classes and stuff in isolation from the main game without making changes to the main file.

To switch to using the testbed_0.cpp file in the Visual Studio project, make sure the `Show All Files` option is enabled in the Solution Explorer to see the real folders. Then, right click on testbed.cpp and select `Include In Project`, then right click on the main.cpp file and select `Exclude From Project`. To switch back, include the main.cpp file and then exclude the testbed_0.cpp file. If you disable `Show All Files` to see the virtual folders, also known as "filters", you'll notice that the entry point file has been moved out of the sampleApp filter, so please make sure to move it back in whenever you do this!

### Code Conventions

- **Pointer Non-Null-Ness to Indicate Existence**: To quickly check whether or not a pointer variable has existing memory attached to it, I like to make the assumption that pointers with any non-nullptr value exist, and that pointers with a value of nullptr do not. Here's a code example of how this could be useful:

```cpp
myClass* someMemory = nullptr; // someMemory starts off with a value of nullptr to indicate that it doesn't exist yet.

// Creating someMemory if it doesn't exist already, and doing nothing otherwise
// I don't normally do this myself because I just avoid calling this type of function twice in the first place,
// but it's still a good example of how you could use this convention.
bool initialize()
{
    if (someMemory != nullptr) // In the event that someMemory already exists...
    {
        return true; // skip creating it again - also return true to indicate that this isn't an error!
    }

    someMemory = new myClass; // Creating someMemory automatically sets it to a non-nullptr value, which automatically signifies that it now exists.

    if (!someMemory->initialize())
    {
        return false; // To contrast with the above check, this IS an error, so the function returns false.
    }

    return true;
}

// Destroying someMemory, but not before checking that it exists first.
void cleanup()
{
    if (someMemory != nullptr) // If someMemory exists... - also prevents an access violation!
    {
        someMemory->cleanup();
        delete someMemory; // This doesn't do anything to the actual value of the pointer, it just deletes the memory.
        someMemory = nullptr; // Set someMemory back to nullptr to indicate that it doesn't exist anymore - also prevents a dangling pointer!
    }
}
```

### Preprocessor Definitions

SIMD:

- `IBEX3D_SIMD_SSE` - Determines whether the SSE instruction is included and used in the build. If this is not specified, basic SISD instructions are used instead. This is currently only used in `vec4::operator +=`, `operator -=`, `operator *=` and `operator /=`, as well as the function `vec4::unsafeDivideBy()`.

Vulkan:

- `IBEX3D_VULKAN_VALIDATION` - Determines whether validation layers (and related helper/extension functions/callbacks) are included and used in the build. If this is not specified, validation layers are disabled, and the functions that use them either do nothing or are excluded entirely. This is currently used in the files `source/ibex3D/vulkan/renderingContext.cpp`, `include/ibex3D/vulkan/vkUtils.h` and `source/ibex3D/vulkan/vkUtils.cpp`.
