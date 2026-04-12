#pragma once
#include <vulkan/vulkan.h>

struct vkPersistentBufferObject
{
	VkBuffer buffer = nullptr;
	VkDeviceMemory bufferMemory = nullptr;
	void* bufferData = nullptr;

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps);
	void cleanup(VkDevice device);

	bool mapBufferData(VkDevice device, VkDeviceSize size);
	void setBufferData(VkDeviceSize size, void* newData);
	void unmapBufferData(VkDevice device);

	bool cmdCopyBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue gfxQueue, VkDeviceSize size, VkBuffer srcBuffer);
};