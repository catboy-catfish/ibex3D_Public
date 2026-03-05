#include <ibexVulkan/vkUtils.h>
#include <ibex3D/utility/utilFunctions.h>

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

#include <limits>
#include <set>
#include <stdio.h>

// ----------------------------------------------------------------------------------------------------
// - Logging ------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

void vk_utilFunctions::printVkError(const char* functionName, const char* message)
{
	printf("VULKAN ERROR - %s: %s\n", functionName, message);
}

void vk_utilFunctions::printVkResultError(VkResult result, const char* functionName, const char* message)
{
	printf("VULKAN ERROR (VkResult: %d) - %s: %s\n", (int)result, functionName, message);
}

// ----------------------------------------------------------------------------------------------------
// - Physical device and swapchain --------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

vk_queueFamilyIndices vk_utilFunctions::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	vk_queueFamilyIndices indices;

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

vk_swapchainSupportInfo vk_utilFunctions::querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	vk_swapchainSupportInfo info;
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

bool vk_utilFunctions::checkPhysicalDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface, bool extensionsSupported)
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

	vk_queueFamilyIndices indices = findQueueFamilies(device, surface);

	bool swapChainSupportAdequate = false;
	if (extensionsSupported)
	{
		vk_swapchainSupportInfo info = querySwapchainSupport(device, surface);
		swapChainSupportAdequate = (!info.formats.empty()) && (!info.presentModes.empty());
	}

	return indices.isComplete() && extensionsSupported && swapChainSupportAdequate;
}

// ----------------------------------------------------------------------------------------------------
// - Swapchain ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

VkSurfaceFormatKHR vk_utilFunctions::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR vk_utilFunctions::choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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

VkExtent2D vk_utilFunctions::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height)
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

		actualExtent.width = clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

// ----------------------------------------------------------------------------------------------------
// - Shader loading -----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

VkShaderModule vk_utilFunctions::createShaderModule(VkDevice logicalDevice, const std::vector<char>& bytecode)
{
	if (bytecode.empty())
	{
		printVkError("createShaderModule()", "Argument \"const std::vector<char>& bytecode\" is empty.");
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
		printVkResultError(result, "createShaderModule()", "Couldn't create the shader module.");
		return nullptr;
	}

	return shaderModule;
}

VkShaderModule vk_utilFunctions::createShaderModuleFromText(VkDevice logicalDevice, const char* fileName)
{
	/*
		Pick up from where you left off at https://youtu.be/Qbs9v1W7St8?t=416
	*/
	
	return nullptr;
}