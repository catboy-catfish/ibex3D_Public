#pragma once
#include <vulkan/vulkan.h>
#include <vector>

struct vkQueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() const
	{
		return (graphicsFamily >= 0) && (presentFamily >= 0);
	}
};

struct vkSwapchainSupportInfo
{
	VkSurfaceCapabilitiesKHR capabilities = {};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class vkUtils
{
public:
	
	// Logging
	static void printVkError(const char* functionName, const char* message);
	static void printVkResultError(VkResult result, const char* functionName, const char* message);

	// Physical device and swapchain
	static vkQueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	static vkSwapchainSupportInfo querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	static bool checkPhysicalDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface, bool extensionsSupported);

	// Swapchain
	static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height);

	// Shader loading
	static VkShaderModule createShaderModule(VkDevice logicalDevice, const std::vector<char>& bytecode);
	static VkShaderModule createShaderModuleFromText(VkDevice logicalDevice, const char* fileName);

	// Buffers
	static bool findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t& outputMemoryType);
	static bool createBuffer(VkPhysicalDevice physicalDevice, VkDevice vkLogicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	static bool copyBuffer(VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	static void destroyBuffer(VkDevice logicalDevice, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
};