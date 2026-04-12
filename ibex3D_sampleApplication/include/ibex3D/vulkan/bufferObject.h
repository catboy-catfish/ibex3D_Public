#pragma once
#include <vulkan/vulkan.h>

struct vkBufferObject
{
	VkBuffer buffer = nullptr;
	VkDeviceMemory bufferMemory = nullptr;

	// ----------------------------------------------------------------------------------------------------

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps);
	void cleanup(VkDevice device);

	bool updateBufferData(VkDevice device, VkDeviceSize size, void* newData);

	bool cmdCopyBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue gfxQueue, VkDeviceSize size, VkBuffer srcBuffer);
};