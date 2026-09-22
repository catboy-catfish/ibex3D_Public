# START HERE

### Table of Contents

- [Assumptions of Prior Experience](#assumptions-of-prior-experience)
  - [Learning resources](#learning-resources)
- [Program Execution](#program-execution)
- [The Entry Point File](#the-entry-point-file)
  - [Testbed](#testbed)
- [Code Conventions](#code-conventions)
- [Preprocessor Definitions](#preprocessor-definitions)
- [Contributing](#contributing)
  - [Documentation](#documentation)
- [To-Do List](#to-do-list)

### Assumptions of Prior Experience

If you work with ibex3D, you are assumed to have basic knowledge of programming in C++ and GLSL, and if you want to implement advanced features, you'll also need intermediate/advanced C++ knowledge as well as graphics programming using the Vulkan API. The ibex3D documentation doesn't go into depth about these very often, and if you have no prior experience with any of them, you might become lost and confused.

In which case, I suggest you use the resources linked in the below section to get started.

##### Learning Resources

For C++, look into:
- [cppreference.com](https://cppreference.com/)
- [cplusplus.com - C++ Language Tutorial](https://cplusplus.com/doc/tutorial/)
- [Learn C++](https://www.learncpp.com/)
- [The Cherno - C++ (YouTube playlist)](https://www.youtube.com/playlist?list=PLlrATfBNZ98dudnM48yfGUldqGD0S4FFb)
- [CodeBeauty - C++ FULL COURSE For Beginners (Learn C++ in 10 hours) (YouTube video)](https://www.youtube.com/watch?v=GQp1zzTwrIg)
- [Mike Shah - The C++ Programming Language (YouTube playlist)](https://www.youtube.com/playlist?list=PLvv0ScY6vfd8j-tlhYVPYgiIyXduu6m-L)
- [Tech With Tim - Learn C++ With Me (YouTube playlist)](https://www.youtube.com/playlist?list=PLzMcBGfZo4-lmGC8VW0iu6qfMHjy7gLQ3)

For Vulkan, look into:
- [Vulkan.org - Learn](https://vulkan.org/learn)
- [Khronos Group - Vulkan Documentation](https://docs.vulkan.org/spec/latest/index.html)
- [Khronos Group - Vulkan Tutorial: Building a Simple Engine](https://docs.vulkan.org/tutorial/latest/Building_a_Simple_Engine/introduction.html)
- [Alexander Overvoorde - Vulkan Tutorial](https://vulkan-tutorial.com/)
- [Khronos Vulkan Tutorial - Introduction](https://docs.vulkan.org/tutorial/latest/00_Introduction.html)
- [Sascha Willems - How to Vulkan in 2026](https://howtovulkan.com/)
- [vblanco20-1 - VkGuide.dev](https://vkguide.dev/)
- [VkGuide.dev - Great resources](https://vkguide.dev/docs/great_resources)
- [zeux.io - Writing an efficient Vulkan renderer](https://zeux.io/2020/02/27/writing-an-efficient-vulkan-renderer/)
- [Brendan Galea - Vulkan (c++) Game Engine Tutorials (YouTube playlist)](https://www.youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR)
- [Cakez - Vulkan Engine - No Libs (YouTube playlist)](https://www.youtube.com/playlist?list=PLFAIgTeqcARkeHm-RimFyKET6IZPxlBSt)
- [Travis Vroman - Vulkan Game Engine Series Written in C (Kohi Game Engine) (YouTube playlist)](https://www.youtube.com/playlist?list=PLv8Ddw9K0JPg1BEO-RS-0MYs423cvLVtj)
- [OGLDEV - Vulkan For Beginners (YouTube playlist)](https://www.youtube.com/playlist?list=PLA0dXqQjCx0RntJy1pqje9uHRF1Z5vZgA)
- [The Graphics Guy - Vulkan (YouTube playlist)](https://www.youtube.com/playlist?list=PLn3eTxaOtL2NH5nbPHMK7gE07SqhcAjmk)
- [Codotaku - Vulkan Tutorial (YouTube playlist)](https://www.youtube.com/playlist?list=PLlKj-4rp1Gz0eBLIcq2wzd8uigFrJduJ-)

For general graphics programming, look into:
- [Joey de Vries - Learn OpenGL](https://learnopengl.com/)
- [RasterTek - DirectX 11 on Windows 10 Tutorials](https://www.rastertek.com/tutdx11win10.html)
- [RasterTek - OpenGL 4.0 on Linux Tutorials](https://www.rastertek.com/tutgl4linux.html)
- [RasterTek - OpenGL 4.0 on Windows Tutorials (historical tutorial)](https://www.rastertek.com/tutgl40.html)
- [OGLDEV - All OpenGL Tutorials (YouTube playlist)](https://www.youtube.com/playlist?list=PLA0dXqQjCx0S04ntJKUftl6OaOgsiwHjA)
- [Victor Gordan - OpenGL Tutorials (YouTube playlist)](https://www.youtube.com/playlist?list=PLPaoO-vpZnumdcb4tZc4x5Q-v7CkrQ6M-)
- [ChiliTomatoNoodle - C++ 3D DirectX Programming](https://www.youtube.com/playlist?list=PLqCJpWy5Fohd3S7ICFXwUomYW0Wv67pDD)
- [PardCode - Game Engine Tutorial Series (YouTube playlist)](https://www.youtube.com/playlist?list=PLv8DnRaQOs5-MR-zbP1QUdq5FL0FWqVzg)
- [PardCode - C++ OpenGL 3D Game Tutorial Series From Scratch (YouTube playlist)](https://www.youtube.com/playlist?list=PLv8DnRaQOs5-TyYnF56YghOxQBNr1VVmF)

I *strongly* recommend that you don't restrict yourself to just these links and resources. Please feel free to look for other C++/Vulkan/graphics programming tutorials - do a quick search for "C++ tutorial", "Vulkan tutorial" or "Graphics programming tutorial" - if you ever get confused, or want to deepen or solidify your existing knowledge. If you have any ideas for better resources, please consider adding or suggesting them in this section!

### Program Execution

The execution for the entire application and game proceeds as follows:

main.cpp -> `i3D_runtime_win32` -> `i3D_application_win32`

The execution of the `i3D_runtime_win32` and `i3D_application_win32` classes are split up into three stages: initialization, runtime, and cleanup. The documentation file for the runtime class contains a detailed explanation of what this means and what the various stages are responsible for.

##### The Entry Point File

The entry point file for the ibex3D sample application is located at `ibex3D_sampleApplication/source/ibex3D/main_win32.cpp`, where the `ibex3D_sampleApplication` folder is located alongside this file's parent folder and the `libraries` folder. The source code of this file looks something like this:

```cpp
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
```

The symbol `ibex3D_entryPoint` is a preprocessor macro in `ibex3D_sampleApplication/include/ibex3D/core/entryPoint.h` which simply switches between `main()` (which has an extra console window for debugging) and `WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)` (with no console window) depending on whether or not `_DEBUG` is specified, which is itself specified only in Debug mode and not in Release mode. The source code of that file looks something like this:

```cpp
#pragma once

// ----------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define ibex3D_entryPoint() main()
#else
// #include "ibexWindows.h"
#define ibex3D_entryPoint() WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
#endif
```

`"ibexWindows.h"` is a Windows-specific utility header that includes `<Windows.h>` in order to access the Win32 API, and a few preprocessor definitions like `WIN32_LEAN_AND_MEAN` and `NOMINMAX`. The source code won't be pasted here in order to keep the file size down, but you can find it in the header file `include/ibex3D/core/ibexWindows.h`.

##### Testbed

In addition to the main entry point file, `source/sampleApp/main.cpp`, the project might also have another entry point .cpp file whose name starts with `testbed_`.
It might be named something like `testbed_simd.cpp` or `testbed_fastgltf.cpp` or `testbed_`whatever`.cpp`.
Anyways, this file is used for testing functions, classes and stuff in isolation from the main game without making changes to the main.cpp file.
Note that this file isn't meant to be used in the final executable.

To switch to using the `testbed_`.cpp file in the Visual Studio project, make sure the `Show All Files` option is enabled in the Solution Explorer to see the real folders.
Then, right click on `testbed_`.cpp and select `Include In Project`, then right click on the main.cpp file and select `Exclude From Project`.
To switch back, include the main.cpp file and then exclude the `testbed_`.cpp file.
If you disable `Show All Files` to see the virtual folders, also known as "filters", you'll notice that the entry point file will have been moved out of the sampleApp filter, so please make sure to move it back in whenever you do this!

### Code Conventions

- **Self-Explanatory Code**: ibex3D's code is intended to be self-explanatory whenever possible, meaning that it is simple and intuitive by design and should ideally be readable enough for any competent programmer to understand without the use of comments.<br><br>I think the best example of this in ibex3D is in the entry point. You can understand what the code below does, right? Hopefully.

```cpp
int i3D_entryPoint()
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
```

- **Class Name Prefixes**: Classes/structs that are specifically a part of ibex3D use the prefix `i3D_` in their names (e.g. `i3D_vec4`), and those that aren't a part of ibex3D don't require it. Classes/structs that use Vulkan use the prefix `vk` (e.g. `vkRenderingContext`). Prefixes can also be mixed, so an ibex3D class that uses Vulkan can be given the prefix `i3D_vk`, for example.

- **Most Failing Functions Return Booleans**: With minor exceptions, all functions that can fail should return a boolean value (`true` or `false`) upon completion. If the function completes execution without any issues, it should return `true`, but if it runs into an error or otherwise fails, it should return `false`. This is done so that the function's output can be evaluated easily.

```cpp
bool someFunction()
{
    if (!someOtherFunction())
    {
        // someOtherFunction() has failed
        return false;
    }

    if (!someOtherOtherFunction())
    {
        // someOtherOtherFunction() has failed
        return false;
    }

    // Both functions were successful
    return true;
}
```

- **Pointer Values**: To quickly check whether or not a pointer variable has existing memory attached to it, I like to make the assumption that pointers with any non-nullptr value have some existing memory attached, and that pointers with a value of nullptr do not. Here's a code example of how this could be useful:

```cpp
myClass* someMemory = nullptr;

bool initialize()
{
    if (someMemory != nullptr)
    {
        return true;
    }

    someMemory = new myClass;

    if (!someMemory->initialize())
    {
        return false;
    }

    return true;
}

void cleanup()
{
    if (someMemory != nullptr)
    {
        someMemory->cleanup();
        delete someMemory; // This deletes the memory, but does nothing to the value of the pointer.
        someMemory = nullptr;
    }
}
```

- **Passing Pointers to Functions**: If you create a function using another function that expects a pointer as one of its parameters, please don't pass a pointer to your function's relative parameter to your target function. Instead, the argument of your function should be a pointer and that should be passed to the target function directly. This is done to avoid confusion.

```cpp
/* DON'T do this! */

bool useObject_wrapper(i3D_object object)
{
    // Function prototype: bool useObject(i3D_object* pObject)
    useObject(&object);

    return true;
}

void main()
{
    i3D_object myObject;
    useObject_wrapper(myObject);
}

/* Do this instead! */

bool useObject_wrapper(i3D_object* pObject)
{
    useObject(pObject);
    return true;
}

void main()
{
    i3D_object myObject;
    useObject_wrapper(&myObject);
}
```

### Preprocessor Definitions

SIMD:

- `I3D_SIMD_SSE` - Determines whether the SSE instruction is included and used in the build. If this is not specified, basic SISD instructions are used instead. This is currently only used in `vec4::operator +=`, `operator -=`, `operator *=` and `operator /=`, as well as the function `vec4::unsafeDivideBy()`.

Vulkan:

- `I3D_VULKAN_VALIDATION` - Determines whether validation layers (and related helper/extension functions/callbacks) are included and used in the build. If this is not specified, validation layers are disabled, and the functions that use them either do nothing or are excluded entirely. This is currently used in the files `source/ibex3D/vulkan/renderingContext.cpp`, `include/ibex3D/vulkan/vkUtils.h` and `source/ibex3D/vulkan/vkUtils.cpp`.

### Contributing

##### Documentation

If you want to create your own documentation, I've personally created a templates folder in order to make your life easier. Files with the `md` file extension should be easily editable with a Markdown text editor like [byxiaozhi's Typedown](https://github.com/byxiaozhi/Typedown) or [this online editor](https://markdownlivepreview.com/). Files ending with `.drawio` must be edited with [draw.io](https://app.diagrams.net/), a free online diagram creation software (note that it requires a browser with JavaScript support).

If you use draw.io to export your diagram as an image, please use the following settings:
- Choose to export your image as a JPEG
- Set the Zoom to 200
- Set the Appearance to Light
- Use the snake_case naming convention (all lowercase words separated by underscores) to name your image file
- Then, export it to the media folder in the parent folder of this file.

![draw.io - Preferred export settings](media/drawio\_export\_convention.jpg)

### To-Do List

For Sam and other developers (if any) to refer to.

Documentation:
- Try to keep up to date if you notice any discrepancies between this file and the source code/APIs!

General/Miscellaneous:
- Finalize documentation for the rest of the engine
- Find out how to move forward and implement what's needed, or focus on pimping the existing code
- Investigate SDL or GLFW as a more stable, cross-platform replacement for the manual Win32 windowing code
- Switch from Visual Studio to something like CMake to become more platform and compiler-independent
- Use meta build systems to exclude platform-specific source code files depending on chosen build platform

C/++ Specific:
- Investigate and implement custom memory allocation functions instead of overriding the new and delete operators
- Look into custom memory allocation/allocators and data alignment

Vulkan-Specific:
- Find a way to compile GLSL code to SPIR-V bytecode at runtime using something like glslang or shaderc (may need to be recompiled with static runtime library?)
- Find a way to allocate the vertex, index and uniform buffer memory with one allocation
- Fix the validation layer error that occurs during render pass creation whenever MSAA is disabled (VUID-VkSubpassDescription-pResolveAttachments-00848)
- Investigate push constants as a replacement for uniform buffers

Model importing:
- The .obj model file format is being ignored by the .gitignore, and I find the vertex/index loading code in vkMeshClass to be hideous since .obj files don't seem to support modern indexing by default. Transition to a better model file format for this like glTF or FBX instead - [vkguide.dev has a tutorial for loading .gltf files using fastgltf, but I'm personally having difficulty understanding it](https://vkguide.dev/docs/new_vkguide/chapter_5).
- Better yet, start using a custom intermediate format for assets that is quickly loadable, efficient and customizable - [The handsome, humble and ever so helpful vkguide.dev has a tutorial for this as well](https://vkguide.dev/docs/extra-chapter/asset_system/).

Math vector types:
- Figure out how to implement SIMD into the vec4 type more efficiently.
- Figure out how to implement SIMD into the vec2 and vec3 types