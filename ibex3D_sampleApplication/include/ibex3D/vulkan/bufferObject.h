#pragma once

#include <vulkan/vulkan.h>

// ----------------------------------------------------------------------------------------------------

// TODO: Figure out how to implement VMA into this
struct i3D_vkBufferObject
{
	VkBuffer buffer = nullptr;
	VkDeviceMemory bufferMemory = nullptr;
	VkDeviceSize bufferSize = 0;

	// ----------------------------------------------------------------------------------------------------

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties);
	void cleanup(VkDevice device);

	bool mapBufferMemory(VkDevice device, VkDeviceSize regionOffset, VkDeviceSize regionSize, VkMemoryMapFlags mapFlags, void** ppData);
	void unmapBufferMemory(VkDevice device);

	bool updateBufferOnce(VkDevice device, VkDeviceSize regionOffset, VkDeviceSize regionSize, VkMemoryMapFlags mapFlags, void* newData);

	bool cmdCopyBuffer(VkDevice device, VkCommandBuffer cmdBuffer, VkBuffer srcBuffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize regionSize);
};