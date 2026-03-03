#pragma once
#include <vulkan/vulkan.h>

#include <vector>

namespace ibex3D_vulkan
{

struct queueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() const
	{
		return (graphicsFamily >= 0) && (presentFamily >= 0);
	}
};

struct swapchainSupportInfo
{
	VkSurfaceCapabilitiesKHR capabilities = {};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class helperFunctions
{
public:
	
	// Logging
	static void printVkError(const char* functionName, const char* message);
	static void printVkResultError(VkResult result, const char* functionName, const char* message);

	// Physical device and swapchain
	static queueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	static swapchainSupportInfo querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	static bool checkPhysicalDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface, bool extensionsSupported);

	// Swapchain
	static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height);

	// Shader loading
	static VkShaderModule createShaderModule(VkDevice logicalDevice, const std::vector<char>& bytecode);
	static VkShaderModule createShaderModuleFromText(VkDevice logicalDevice, const char* fileName);
};

}