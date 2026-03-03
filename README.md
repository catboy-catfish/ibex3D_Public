### Important notice

Despite the project having a Win32 platform configuration, it only supports building for 64-bit. This is because multiple parts of the code only work on x64, and the Vulkan SDK version 1.4.341.0 seems to only support x64 as well. I'd like to support 32-bit platforms eventually, but not right now.

### Readme

This is the repository for the game engine I'm trying to create, ibex3D. It is written in C++, uses the Win32 API for windowing, and the Vulkan API to render graphics. It's based off of the Vulkan Tutorial by Alexander Overvoorde, but I've made some architectural changes to make it more my style.

This is not even close to finished. The main thing I want to draw attention to here is the presence of a memory leak that occurs after the application exits, because it seems some resources are not freed properly. I haven't been able to find out the cause or location of the memory leak. The project was originally developed using the Vulkan SDK version 1.3.296.0, where I originally noticed this issue. I've now switched it to use version 1.4.341.0 as a last ditch effort to see if it's not on the SDK's part, but this has made it even worse for some reason, so I think it's an error on my part that I'm not seeing.

Aside from the memory leak, I'm pretty sure this codebase has multiple other issues as well. Information on that is welcomed, but I'd like the memory leak to be the main focus of the conversation.

### How to Build and Run

1. Open the project/solution in Visual Studio 2022.
2. Build the project (ideally using the Debug configuration) and make sure it compiles with no errors.
3. Debug the project using the Diagnostic Tools.
4. Close the program window (not the console window) and pay attention to the Process Memory. You'll see it slightly goes down step by step, but then it stays at that level until the program exits.
5. When you want to exit the application altogether, there should be a "Press any key to continue . . . " line in the command prompt window. Press any key to exit the application.

##### Shader Compilation

The shaders in the "shaders" folder have already been compiled to SPIR-V, and those SPIR-V files worked for me on two machines, but I'm not sure if they would work for you (I don't know if SPIR-V compilation output depends on your hardware/graphics drivers or not). If it doesn't work for you, try following these steps:

1. Go to "ibex3D_sampleApplication/shaders/" and edit the "compile.bat" file inside.
2. Replace both occurences of "D:/VulkanSDK/1.4.341.1/Bin/glslc.exe" with the path to glslc.exe in your Vulkan SDK installation.
3. Run "compile.bat" and see if you don't get any errors.

### Requirements

- Windows 10 or 11 - This codebase currently only supports the Windows operating system because it uses Win32-specific code. It's been tested on versions 10 and 11, but older/other versions might work as well.

- Vulkan SDK - This codebase uses the Vulkan SDK version 1.4.341.0. You might be able to run it on other versions (I've personally run it using version 1.3.296.0), but it's necessary that you have the Vulkan SDK installed on your machine.

- Visual Studio - This codebase uses Microsoft Visual Studio Community 2022 version 17.14.27 and the MSVC compiler, so you need those installed in order to be able to build and run it.