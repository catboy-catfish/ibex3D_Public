### Notice: assets/models/viking_room.obbj

I renamed the model file `viking_room.obbj` to get around the .gitignore, which seems to ignore all .obj files no matter if they're from Visual Studio or 3D modeling software. Please rename it to `viking_room.obj` for the application to run properly.

### Readme

This is the repository for the game engine I'm trying to create, ibex3D. It is written in C++, uses the Win32 API for windowing, and the Vulkan API to render graphics. It's based off of the Vulkan Tutorial by Alexander Overvoorde, but I've made some architectural changes to make it more my style. I want to look into other tutorials and learn more, so that I can eventually keep improving it until it's capable of what I want to make with it in the future.

This is not even close to finished. I'm pretty sure this codebase has multiple issues that I'm not seeing yet, aside from the frankly horrible architecture.

### Requirements

- Windows 10 or 11 (x64) - Currently only supports the Windows operating system because it uses Win32-specific code. It's been tested on versions 10 and 11, but older/other versions might work as well. However, you NEED to run this with a machine with 64-bit support, as it does not have any 32-bit support.

- Vulkan - Exclusively uses the Vulkan API for graphics right now. You'll need a graphics card with Vulkan support in order to be able to run it. This should be pretty common as Vulkan is widely supported by most modern GPUs.

- Vulkan SDK - Uses the Vulkan SDK version 1.4.341.0. You might be able to run it on other versions (I've personally run it using version 1.3.296.0), but it's necessary that you have the Vulkan SDK installed on your machine.

- Visual Studio 2022 - Uses Microsoft Visual Studio Community 2022 version 17.14.27 and the MSVC compiler, so you need those installed in order to be able to build and run it. I want to switch it to something like CMake one day, but I have no idea how to use it right now.

##### Shader Compilation

The shaders in the `ibex3D_sampleApplication/assets/shaders` folder have already been compiled to SPIR-V, and those SPIR-V files worked for me on two machines, but I'm not sure if they would work for you (I don't know if SPIR-V compilation output depends on your hardware and graphics drivers or not). If it doesn't work for you, try following these steps:

1. Go to `ibex3D_sampleApplication/assets/shaders/` and edit the `compile.bat` file inside.
2. Replace both occurences of `D:/VulkanSDK/1.4.341.1/Bin/glslc.exe` with the absolute path to glslc.exe in your Vulkan SDK installation (this would be `VulkanSDK/(version)/Bin/glslc.exe`).
3. Run `compile.bat` and see if you don't get any errors in the command line.
