#pragma once
#include <vulkan/vulkan.h>

#include <vector>

// ----------------------------------------------------------------------------------------------------

struct vkSwapchainObject
{
	VkSwapchainKHR swapchain = nullptr;

	VkImage colorImage = nullptr;
	VkImageView colorImageView = nullptr;
	VkDeviceMemory colorImageMemory = nullptr;

	VkImage depthImage = nullptr;
	VkImageView depthImageView = nullptr;
	VkDeviceMemory depthImageMemory = nullptr;

	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	uint32_t imageCount = 0;
	VkFormat imageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D imageExtent = { 0, 0 };

	// ----------------------------------------------------------------------------------------------------

	bool initSwapchain(VkDevice device, VkPhysicalDevice physDevice, VkSurfaceKHR surface, int wndWidth, int wndHeight, bool vSync);
	bool initColorResources(VkDevice device, VkPhysicalDevice physDevice, VkSampleCountFlagBits msaaSamples);
	bool initDepthResources(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, VkSampleCountFlagBits msaaSamples);

	void cleanupSwapchain(VkDevice device);
	void cleanupColorResources(VkDevice device);
	void cleanupDepthResources(VkDevice device);
};