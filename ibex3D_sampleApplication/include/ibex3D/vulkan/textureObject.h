#pragma once

#include <vulkan/vulkan.h>

struct vkTextureObject
{
	VkImage image = nullptr;
	VkDeviceMemory imageMemory = nullptr;
	VkImageView imageView = nullptr;
	VkSampler sampler = nullptr;

	uint32_t mipLevels = 0;

	bool initImageAndView(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* imgFilePath);
	bool initSampler(VkDevice device, VkPhysicalDevice physDevice);

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* imgFilePath);
	void cleanup(VkDevice device);
};