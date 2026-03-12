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

	return indices.isComplete() && extensionsSupported && swapChainSupportAdequate && deviceFeatures.samplerAnisotropy;
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

// ----------------------------------------------------------------------------------------------------
// - Command buffers ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

VkCommandBuffer vkUtils::beginSingleTimeCommands(VkDevice logicalDevice, VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = nullptr;

	VkResult result = vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

	if (result != VK_SUCCESS)
	{
		printVkResultError(result, "vkUtils::beginSingleTimeCommands()", "Couldn't allocate the command buffer.");
		return nullptr;
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	// TODO: Add error checking?
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void vkUtils::endSingleTimeCommands(VkDevice logicalDevice, VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	// TODO: Add error checking to all of these?
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

// ----------------------------------------------------------------------------------------------------
// - Buffers ------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

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
	
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(logicalDevice, commandPool);

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	
	endSingleTimeCommands(logicalDevice, commandBuffer, commandPool, graphicsQueue);

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

bool vkUtils::createImage(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;	// Optional.

	VkResult result = vkCreateImage(logicalDevice, &imageInfo, nullptr, &image);

	if (result != VK_SUCCESS)
	{
		printVkResultError(result, "vkUtils::createImage()", "Couldn't create the image.");
		return false;
	}

	VkMemoryRequirements memRequirements = {};
	vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

	uint32_t memoryType = 0;

	if (!vkUtils::findMemoryType
	(
		physicalDevice,
		memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		memoryType
	))
	{
		vkUtils::printVkError("vkUtils::createImage()", "Couldn't find the memory type for the image memory.");
		return false;
	}

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memoryType;

	result = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkUtils::createImage()", "Couldn't allocate the image memory.");
		return false;
	}

	vkBindImageMemory(logicalDevice, image, imageMemory, 0);
	return true;
}

VkImageView vkUtils::createImageView(VkDevice logicalDevice, VkImage image, VkFormat format)
{
	VkImageView imageView = nullptr;

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkResult result = vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkUtils::createImageView()", "Couldn't create the image view.");
		return nullptr;
	}

	return imageView;
}

bool vkUtils::copyBufferToImage(VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(logicalDevice, commandPool);
	
	VkBufferImageCopy copyRegion = {};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;
	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.imageOffset = { 0, 0, 0 };
	copyRegion.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage
	(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&copyRegion
	);

	endSingleTimeCommands(logicalDevice, commandBuffer, commandPool, graphicsQueue);
	return true;
}

bool vkUtils::transitionImageLayout(VkDevice logicalDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	// TODO: Error checking?
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(logicalDevice, commandPool);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;		// This image doesn't have any mipmaps
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;	// This image is not an array
	barrier.subresourceRange.layerCount = 1;
	
	VkPipelineStageFlags srcStage, dstStage;
	
	if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else
	{
		if ((oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			printVkError("vkUtils::transitionImageLayout()", "Unsupported transition. Supported transitions include \"UNDEFINED -> TRANSFER_DST_OPTIMAL\" and \"TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL\".");
			
			vkEndCommandBuffer(commandBuffer);
			vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
			
			return false;
		}
	}

	barrier.srcAccessMask = 0;						// TODO
	barrier.dstAccessMask = 0;

	vkCmdPipelineBarrier
	(
		commandBuffer,
		srcStage,
		dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommands(logicalDevice, commandBuffer, commandPool, graphicsQueue);
	return true;
}