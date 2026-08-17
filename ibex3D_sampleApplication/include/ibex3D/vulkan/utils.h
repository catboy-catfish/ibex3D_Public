#pragma once

#include <vulkan/vulkan.h>

#include <vector>

// ----------------------------------------------------------------------------------------------------

struct i3D_vkQueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() const
	{
		return (graphicsFamily >= 0) && (presentFamily >= 0);
	}
};

struct i3D_vkSwapchainSupportInfo
{
	VkSurfaceCapabilitiesKHR capabilities = {};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

// ----------------------------------------------------------------------------------------------------

class i3D_vkUtils
{
public:
	// Physical device and swapchain
	static i3D_vkQueueFamilyIndices findQueueFamilies(VkPhysicalDevice physDevice, VkSurfaceKHR surface);
	static i3D_vkSwapchainSupportInfo querySwapchainSupport(VkPhysicalDevice physDevice, VkSurfaceKHR surface);

	// Shader loading
	static VkShaderModule createShaderModuleFromSPIRV(VkDevice device, const char* filePath);
	static VkShaderModule createShaderModuleFromGLSL(VkDevice device, const char* filePath); // NOT IMPLEMENTED!!!

	// Command buffers
	static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool pool);
	static void endSingleTimeCommands(VkDevice device, VkCommandPool pool, VkQueue gfxQueue, VkCommandBuffer buffer);

	// Buffers
	static bool findMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags memPropertyFlags, uint32_t& outputMemoryType);

	// Formats
	static bool findSupportedFormat(VkPhysicalDevice physDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkFormat& outFormat);
	static bool findDepthFormat(VkPhysicalDevice physDevice, VkFormat& outFormat);
	static bool formatHasStencilComponent(VkFormat format);

	// Images
	static bool createImage(VkDevice device, VkPhysicalDevice physDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memProperties, VkImage& image, VkDeviceMemory& imageMem);
	static VkImageView createImageView(VkDevice device, VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspect);
	static bool copyBufferToImage(VkDevice device, VkCommandBuffer cmdBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	static bool transitionImageLayout(VkDevice device, VkCommandBuffer cmdBuffer, VkImage image, uint32_t mipLevels, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	static bool generateMipmaps(VkDevice device, VkPhysicalDevice physDevice, VkCommandBuffer cmdBuffer, VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
};