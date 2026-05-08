#pragma once
#include <vulkan/vulkan.h>

struct vkBufferObject
{
	VkBuffer buffer = nullptr;
	VkDeviceMemory bufferMemory = nullptr;
	VkDeviceSize bufferSize = 0;

	// ----------------------------------------------------------------------------------------------------

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps);
	void cleanup(VkDevice device);

	bool updateBufferData(VkDevice device, void* newData);

	bool cmdCopyBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue gfxQueue, VkBuffer srcBuffer, VkDeviceSize srcBufSize);
};