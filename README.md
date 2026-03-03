### Important notice

Despite the project having a Win32 platform configuration, it only supports building for 64-bit. This is because multiple parts of the code only work on x64, and the Vulkan SDK version 1.4.341.0 seems to only support x64 as well. I'd like to support 32-bit platforms eventually, but not right now.

### Readme

This is the repository for the game engine I'm trying to create, ibex3D. It is written in C++, uses the Win32 API for windowing, and the Vulkan API to render graphics. It's based off of the Vulkan Tutorial by Alexander Overvoorde, but I've made some architectural changes to make it more my style.

This is not even close to finished. The main thing I want to draw attention to here is the presence of a memory leak that occurs after the application exits, because it seems some resources are not freed properly. I haven't been able to find out the cause or location of the memory leak. The project was originally developed using the Vulkan SDK version 1.3.296.0, where I originally noticed this issue. I've now switched it to use version 1.4.341.0 as a last ditch effort to see if it's not on the SDK's part, but this has made it even worse for some reason, so I think it's an error on my part that I'm not seeing.

Aside from the memory leak, I'm pretty sure this codebase has multiple other issues as well, I'm aware of that. Information on those issues is welcomed, but I want the memory leak to be the main focus of this conversation. I'd appreciate if you guys could help me out solving this and offering advice.

### How to Run/Build

##### Shader Compilation

### Requirements