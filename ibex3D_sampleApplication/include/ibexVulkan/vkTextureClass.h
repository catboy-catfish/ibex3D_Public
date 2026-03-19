#pragma once

#include <vulkan/vulkan.h>

struct vkTextureClass
{
	VkImage image = nullptr;
	VkDeviceMemory imageMemory = nullptr;
	VkImageView imageView = nullptr;
	VkSampler sampler = nullptr;

	uint32_t mipLevels = 0;

	bool initImageAndView(VkDevice device, VkPhysicalDevice physDevice, const char* imgFilePath, VkCommandPool cmdPool, VkQueue gfxQueue);
	bool initSampler(VkDevice device, VkPhysicalDevice physDevice);

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, const char* imgFilePath, VkCommandPool cmdPool, VkQueue gfxQueue);
	void cleanup(VkDevice device);
};