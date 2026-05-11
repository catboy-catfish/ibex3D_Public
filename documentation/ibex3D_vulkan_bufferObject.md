# vkBufferObject - Basic overview

- Header file: `include/ibex3D/vulkan/bufferObject.h`
- Source file: `source/ibex3D/vulkan/bufferObject.cpp`

### Table of Contents

- [Description](#description)
- [Functions](#functions)
- [Member variables](#member-variables)
- [Remarks](#remarks)
- [Examples](#examples)
- [To-do list](#to-do-list)

### Description

`vkBufferObject` is a helper struct containing a `VkBuffer` and its corresponding `VkDeviceMemory`. It also includes functions for initializing/cleaning up the buffer and assigning its memory.

This struct should ideally not be used for buffers which will be updated frequently due to the runtime cost of mapping and unmapping. For a variant designed for persistent mapping, which is better suited for this purpose, see `vkPersistentBufferObject` (no documentation yet).

### Functions

```
DISCLAIMER: The officiai Vulkan SDK gives the prefix "Vk" (with a capital V) to classes/structs, and the prefix "vk" (with a lowercase v) to functions. The custom Vulkan helper classes/structs as part of ibex3D have the prefix "vk" (with a lowercase v), and the functions have no "Vk" or "vk" prefix.
```

`bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps)`
- **Note: If this function fails, the buffer and its memory won't be freed automatically. You'll have to manually call the `cleanup()` function to do this.**
- Attempts to create the buffer, allocate the buffer memory, and then bind the buffer memory. This function takes in the logical device, the physical device, the total size of the buffer, and its usage and memory properties flags.
- Returns true if all stages of the function are successful, and false otherwise.

`void cleanup(VkDevice device)`
- Destroys the buffer and frees the buffer memory using the logical device, if they are initialized/allocated.

`bool updateBufferData(VkDevice device, void* newData)`
- Attempts to map the buffer memory into application address space, update the buffer memory with the new data passed in, and then unmap the buffer memory. This function takes the logical device and a pointer to the new buffer data.
- Note that mapping and unmapping in Vulkan has a runtime cost. This function (and by extension, this entire struct) shouldn't be used in scenarios where the buffer data is frequently updated. You may want to consider using the `vkPersistentBufferObject` instead, which is designed for persistent mapping.
- Returns true if the buffer memory is mapped successfully, and false otherwise.

`bool cmdCopyBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue gfxQueue, VkBuffer srcBuffer, VkDeviceSize srcBufSize)`
- Copies the data from another buffer (the source) into the member variable `buffer` (the destination). This function takes the logical device, the command pool, the graphics queue, the source buffer and the size of the source buffer.
- Note that the size of `srcBuffer` should ideally be the same as the destination `bufferSize`. I haven't considered the possibility of the source and destination buffers having different sizes, but for now, I reccommend you ensure that the size of both buffers are the same unless you know what you are doing.
- Returns true if the command buffer can successfully begin, and false otherwise.

### Member variables

```
DISCLAIMER: Tell users about anything urgent or important about these variables they should know before using them.
Remove this block if you don't have anything to say.
```

`VkBuffer buffer`
- The variable for the internal buffer, or the `VkBuffer`.

`VkDeviceMemory bufferMemory`
- The variable for the buffer memory, or the `VkDeviceMemory`.

`VkDeviceSize bufferSize`
- The size of the internal buffer. This is set by the `initialize()` function and its `size` parameter, and set back to 0 by the `cleanup()` function.

### Remarks

Compared to the non-Vulkan classes, I'm having some trouble fully understanding the Vulkan API at the moment, and therefore I'm probably not doing a good job at explaining it. If you're more experienced in Vulkan than I am and see potential ways that the documentation could be enhanced, please make suggestions or modify this document to accomodate that!

In the meantime, I suggest you take a look at the Vulkan API functions in the source code and search for their official documentation. Here are the functions that are used in this class:

- [vkCreateBuffer](https://docs.vulkan.org/refpages/latest/refpages/source/vkCreateBuffer.html)
- [vkGetBufferMemoryRequirements](https://docs.vulkan.org/refpages/latest/refpages/source/vkGetBufferMemoryRequirements.html)
- [vkAllocateMemory](https://docs.vulkan.org/refpages/latest/refpages/source/vkAllocateMemory.html)
- [vkBindBufferMemory](https://docs.vulkan.org/refpages/latest/refpages/source/vkBindBufferMemory.html)
- [vkMapMemory](https://docs.vulkan.org/refpages/latest/refpages/source/vkMapMemory.html)
- [vkUnmapMemory](https://docs.vulkan.org/refpages/latest/refpages/source/vkUnmapMemory.html)
- [vkCmdCopyBuffer](https://docs.vulkan.org/refpages/latest/refpages/source/vkCmdCopyBuffer.html)
- [vkDestroyBuffer](https://docs.vulkan.org/refpages/latest/refpages/source/vkDestroyBuffer.html)
- [vkFreeBufferMemory](https://docs.vulkan.org/refpages/latest/refpages/source/vkFreeMemory.html)

### Examples

How to initialize and cleanup a combined vertex/index buffer and staging buffer (as used in the file `include/ibex3D/vulkan/meshObject.cpp`)

```cpp
// Initialization
bool vkMeshObject::initVertexIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue)
{
    vtxBufferSize = sizeof(vertices[0]) * vertices.size();

    VkDeviceSize idxBufferSize = sizeof(indices[0]) * indices.size();
    VkDeviceSize combinedBufferSize = vtxBufferSize + idxBufferSize;

    vkBufferObject stagingBuffer;

    if (!stagingBuffer.initialize
    (
        device,
        physDevice,
        combinedBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    ))
    {
        vkUtils::printVkError("vkMeshObject::initVertexIndexBuffer()", "Couldn't create the staging buffer.");
        return false;
    }

    void* data;
    vkMapMemory(device, stagingBuffer.bufferMemory, 0, combinedBufferSize, 0, &data);

    memcpy(data, vertices.data(), vtxBufferSize);
    memcpy(static_cast<char*>(data) + vtxBufferSize, indices.data(), idxBufferSize);

    vkUnmapMemory(device, stagingBuffer.bufferMemory);

    if (!vtxIdxBuffer.initialize
    (
        device,
        physDevice,
        combinedBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    ))
    {
        vkUtils::printVkError("vkMeshObject::initVertexIndexBuffer()", "Couldn't create the combined vertex-index buffer.");
        stagingBuffer.cleanup(device);
        return false;
    }

    if (!vtxIdxBuffer.cmdCopyBuffer(device, cmdPool, gfxQueue, combinedBufferSize, stagingBuffer.buffer))
    {
        vkUtils::printVkError("vkMeshObject::initVertexIndexBuffer()", "Couldn't copy the staging memory to the combined vertex-index buffer.");
        stagingBuffer.cleanup(device);
        return false;
    }

    stagingBuffer.cleanup(device);
    return true;
}

// Cleanup
void vkMeshObject::cleanup(VkDevice device)
{    
    vtxIdxBuffer.cleanup(device);
}
```

### To-do list

- Find a way to account for the size of the source and destination buffers being different in the `cmdCopyBuffer()` function.
- Improve the documentation for this class.
