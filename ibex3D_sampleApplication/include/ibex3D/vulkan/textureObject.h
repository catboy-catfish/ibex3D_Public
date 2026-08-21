#pragma once

#include <vulkan/vulkan.h>

#include <thirdparty/vma/vk_mem_alloc.h>

// ----------------------------------------------------------------------------------------------------

struct i3D_vkTextureObject
{
	VkImage image = nullptr;
	VkDeviceMemory imageMemory = nullptr;
	VmaAllocation allocation = nullptr;
	VkImageView imageView = nullptr;
	VkSampler sampler = nullptr;

	uint32_t mipLevels = 0;

	// ----------------------------------------------------------------------------------------------------

	bool initImage(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* imgFilePath);
	bool initImage_VMA(VkDevice device, VkPhysicalDevice physDevice, VmaAllocator allocator, VkCommandPool cmdPool, VkQueue gfxQueue, const char* imgFilePath);
	bool initImageView(VkDevice device);
	bool initSampler(VkDevice device, VkPhysicalDevice physDevice);

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* imgFilePath);
	bool initialize_VMA(VkDevice device, VkPhysicalDevice physDevice, VmaAllocator allocator, VkCommandPool cmdPool, VkQueue gfxQueue, const char* imgFilePath);
	void cleanup(VkDevice device, VmaAllocator allocator);
};