ibex3D doesn't have any license of its own right now, but when it's ready, I want to put it under something like the MIT license. Whatever license I pick for it, it is free and open-source software and I hope that never changes.

---

ibex3D uses multiple third-party libraries with various licenses:
- [Vulkan SDK 1.4.341.1](https://www.google.com/search?q=vulkan+sdk&ie=UTF-8) by LunarG - features components under [multiple licenses](https://vulkan.lunarg.com/license/#/release/record/8028433/linux)
- [Vulkan Memory Allocator](https://gpuopen.com/vulkan-memory-allocator/) by Advanced Micro Devices - under the MIT license
- [OpenGL Mathematics (GLM)](https://github.com/g-truc/glm) by G-Truc Creation - under the Happy Bunny license (modified MIT license)
- [stb](https://github.com/nothings/stb) by Sean T. Barrett (nothings on GitHub) - under the MIT license (Alternative A)
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) by Syoyo Fujita - under the MIT license
- [fastgltf](https://github.com/spnda/fastgltf) by Sean Apeler - under the MIT license

and the following assets:
- [Material ball in 3D-Coat](https://skfb.ly/FrVp) by 3d-coat on Sketchfab - under [Creative Commons Attribution 4.0](http://creativecommons.org/licenses/by/4.0/).

A copy of the licenses for almost every third-party library used can be found at path `ibex3D_sampleApplication/include/thirdparty/`, except for the Vulkan SDK, which contains many different licenses that I can't be bothered to include in this repository right now.