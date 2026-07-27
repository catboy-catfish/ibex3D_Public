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
		return (graphicsFamily > -1) && (presentFamily > -1);
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

	// Validation layers
#ifdef I3D_VULKAN_VALIDATION
	static VkResult createDebugMessenger
	(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger
	);

	static void destroyDebugMessenger
	(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator
	);

	static VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback
	(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);
#endif

	// Physical device and swapchain
	static i3D_vkQueueFamilyIndices findQueueFamilies(VkPhysicalDevice physDevice, VkSurfaceKHR surface);
	static i3D_vkSwapchainSupportInfo querySwapchainSupport(VkPhysicalDevice physDevice, VkSurfaceKHR surface);
	static VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physDevice);
	static int ratePhysicalDeviceSuitability(VkPhysicalDevice physDevice, VkSurfaceKHR surface, bool extSupport);

	// Swapchain
	static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availableModes, bool vSync);
	static VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& surfaceCaps, int width, int height);

	// Shader loading
	static VkShaderModule createShaderModuleFromSPIRV(VkDevice device, const std::vector<char>& spirvBytecode);
	static VkShaderModule createShaderModuleFromGLSL(VkDevice device, const char* fileName); // NOT IMPLEMENTED!!!

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