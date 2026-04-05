# TO-DO LIST:

General/Miscellaneous:

- Add documentation
- Focus on styling up the engine code until you have the balls to make actual progress
- Switch from Visual Studio to something like CMake to become more platform and compiler-independent
- Use meta build systems to exclude source code files depending on build platform
- Investigate GLFW as a more stable, cross-platform replacement for the manual Win32 windowing code

C++:

- Investigate and implement custom memory allocation functions instead of overriding the new and delete operators
- Look into custom memory allocation and data alignment in C++

Vulkan:

- Fix the validation layer error that occurs during render pass creation whenever MSAA is disabled (VUID-VkSubpassDescription-pResolveAttachments-00848)
- Find a way to compile GLSL code to SPIR-V bytecode at runtime with something like glslang or shaderc
- Find a way to allocate the vertex, index and uniform buffer memory with one allocation
- Find a way to consolidate the vertex, index and uniform buffers into one buffer using offsets
- Investigate push constants as a replacement for uniform buffers

Math vector types:

- Figure out how to implement SIMD into the vec4 +, -, * and / operators without it being *slower* than SISD
- Implement SIMD into the vec2 and vec3 structs as well

Model importing:

- The .obj model is being ignored by the .gitignore and I find the vertex/index loading code in vkMeshClass to be hideous
- Switch to a better model file format like glTF or FBX instead