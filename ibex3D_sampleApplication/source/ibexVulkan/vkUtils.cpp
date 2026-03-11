#include <ibexVulkan/vkUtils.h>
#include <ibex3D/utility/utilFunctions.h>

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

#include <algorithm>
#include <limits>
#include <set>
#include <stdio.h>

// ----------------------------------------------------------------------------------------------------
// - Extension functions ------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

#ifdef IBEX3D_VULKAN_USE_VALIDATION_LAYERS
VkResult vkExtFunctions::CreateDebugUtilsMessengerEXT
(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger
)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void vkExtFunctions::DestroyDebugUtilsMessengerEXT
(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator
)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}
#endif

// ----------------------------------------------------------------------------------------------------
// - Logging ------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

void vkUtils::printVkError(const char* functionName, const char* message)
{
	printf("VULKAN ERROR - %s: %s\n", functionName, message);
}

void vkUtils::printVkResultError(VkResult result, const char* functionName, const char* message)
{
	printf("VULKAN ERROR (VkResult: %d) - %s: %s\n", (int)result, functionName, message);
}

// ----------------------------------------------------------------------------------------------------
// - Physical device and swapchain --------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

vkQueueFamilyIndices vkUtils::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	vkQueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	for (int i = 0; i < queueFamilies.size(); i++)
	{
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport == VK_TRUE)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}
	}

	return indices;
}

vkSwapchainSupportInfo vkUtils::querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	vkSwapchainSupportInfo info;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &info.capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		info.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, info.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		info.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, info.presentModes.data());
	}

	return info;
}

bool vkUtils::checkPhysicalDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface, bool extensionsSupported)
{
	VkPhysicalDeviceProperties deviceProperties = {};
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		return false;
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	if (!deviceFeatures.geometryShader)
	{
		return false;
	}

	vkQueueFamilyIndices indices = findQueueFamilies(device, surface);

	bool swapChainSupportAdequate = false;
	if (extensionsSupported)
	{
		vkSwapchainSupportInfo info = querySwapchainSupport(device, surface);
		swapChainSupportAdequate = (!info.formats.empty()) && (!info.presentModes.empty());
	}

	return indices.isComplete() && extensionsSupported && swapChainSupportAdequate;
}

// ----------------------------------------------------------------------------------------------------
// - Swapchain ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

VkSurfaceFormatKHR vkUtils::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR vkUtils::choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D vkUtils::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height)
{
	if (capabilities.currentExtent.width != UINT_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

// ----------------------------------------------------------------------------------------------------
// - Shader loading -----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

VkShaderModule vkUtils::createShaderModule(VkDevice logicalDevice, const std::vector<char>& bytecode)
{
	if (bytecode.empty())
	{
		printVkError("vkUtils::createShaderModule()", "Argument \"const std::vector<char>& bytecode\" is empty.");
		return nullptr;
	}

	VkShaderModuleCreateInfo moduleInfo = {};
	moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.codeSize = bytecode.size();
	moduleInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

	VkShaderModule shaderModule = nullptr;
	VkResult result = vkCreateShaderModule(logicalDevice, &moduleInfo, nullptr, &shaderModule);

	if (result != VK_SUCCESS)
	{
		printVkResultError(result, "vkUtils::createShaderModule()", "Couldn't create the shader module.");
		return nullptr;
	}

	return shaderModule;
}

VkShaderModule vkUtils::createShaderModuleFromText(VkDevice logicalDevice, const char* fileName)
{
	/*
		Pick up from where you left off at https://youtu.be/Qbs9v1W7St8?t=416
	*/
	
	return nullptr;
}

bool vkUtils::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t& outputMemoryType)
{
	VkPhysicalDeviceMemoryProperties memProperties = {};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && ((memProperties.memoryTypes[i].propertyFlags & properties) == properties))
		{
			outputMemoryType = i;
			return true;
		}
	}

	vkUtils::printVkError("vkUtils::findMemoryType()", "Couldn't find any suitable memory type.");
	return false;
}

bool vkUtils::createBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkUtils::createBuffer()", "Couldn't create the buffer.");
		return false;
	}

	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

	uint32_t memoryType = 0;
	if (!findMemoryType
	(
		physicalDevice,
		memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		memoryType
	))
	{
		return false;
	}

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = memoryType;

	result = vkAllocateMemory(logicalDevice, &allocateInfo, nullptr, &bufferMemory);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkUtils::createBuffer()", "Couldn't allocate the buffer memory.");
		return false;
	}

	vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);

	return true;
}

bool vkUtils::copyBuffer(VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	// TODO: Implement error checking?
	
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = nullptr;
	vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
	commandBuffer = nullptr;

	return true;
}

void vkUtils::destroyBuffer(VkDevice logicalDevice, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	if (buffer != nullptr)
	{
		vkDestroyBuffer(logicalDevice, buffer, nullptr);
		buffer = nullptr;
	}

	if (bufferMemory != nullptr)
	{
		vkFreeMemory(logicalDevice, bufferMemory, nullptr);
		bufferMemory = nullptr;
	}
}