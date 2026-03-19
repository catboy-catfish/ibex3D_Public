#pragma once
#include <vulkan/vulkan.h>
#include <vector>

struct vkQueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() const
	{
		return (graphicsFamily > -1) && (presentFamily > -1);
	}
};

struct vkSwapchainSupportInfo
{
	VkSurfaceCapabilitiesKHR capabilities = {};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class vkExtFunctions
{
public:
#ifdef IBEX3D_VULKAN_USE_VALIDATION_LAYERS
	static VkResult CreateDebugUtilsMessengerEXT
	(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger
	);

	static void DestroyDebugUtilsMessengerEXT
	(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator
	);
#endif
};

class vkUtils
{
public:

	// Logging
	static void printVkError(const char* functionName, const char* message);
	static void printVkResultError(VkResult result, const char* functionName, const char* message);

	// Validation layers
#ifdef IBEX3D_VULKAN_USE_VALIDATION_LAYERS
	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback
	(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);
#endif

	// Physical device and swapchain
	static vkQueueFamilyIndices findQueueFamilies(VkPhysicalDevice physDevice, VkSurfaceKHR surface);
	static vkSwapchainSupportInfo querySwapchainSupport(VkPhysicalDevice physDevice, VkSurfaceKHR surface);
	static VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physDevice);
	static int ratePhysicalDeviceSuitability(VkPhysicalDevice physDevice, VkSurfaceKHR surface, bool extSupport);

	// Swapchain
	static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availableModes);
	static VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& surfaceCaps, int width, int height);

	// Shader loading
	static VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& spirvBytecode);
	static VkShaderModule createShaderModuleFromText(VkDevice device, const char* fileName);			// NOT IMPLEMENTED!!!

	// Command buffers
	static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool pool);
	static void endSingleTimeCommands(VkDevice device, VkCommandPool pool, VkQueue gfxQueue, VkCommandBuffer buffer);

	// Buffers
	static bool findMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags memPropertyFlags, uint32_t& outputMemoryType);
	static bool createBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags memProperties, VkBuffer& buffer, VkDeviceMemory& bufferMem);
	static bool copyBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue gfxQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);
	static void destroyBuffer(VkDevice device, VkBuffer& buffer, VkDeviceMemory& bufferMem);

	// Depth buffers
	static VkFormat findSupportedFormat(VkPhysicalDevice physDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	static VkFormat findDepthFormat(VkPhysicalDevice physDevice);
	static bool formatHasStencilComponent(VkFormat format);

	// Images
	static bool createImage(VkDevice device, VkPhysicalDevice physDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memProperties, VkImage& image, VkDeviceMemory& imageMem);
	static VkImageView createImageView(VkDevice device, VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspect);
	static bool copyBufferToImage(VkDevice device, VkCommandPool cmdPool, VkQueue gfxQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	static bool transitionImageLayout(VkDevice device, VkCommandPool cmdPool, VkQueue gfxQueue, VkImage image, uint32_t mipLevels, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	static bool generateMipmaps(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
};