#include <ibex3D/vulkan/utils.h>

#include <ibex3D/core/logger.h>
#include <ibex3D/core/fileAccess.h>

#include <algorithm>
#include <limits>
#include <set>
#include <string>

#include <vulkan/vk_enum_string_helper.h>

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

// - Physical device and swapchain --------------------------------------------------------------------

i3D_vkQueueFamilyIndices i3D_vkUtils::findQueueFamilies(VkPhysicalDevice physDevice, VkSurfaceKHR surface)
{
	i3D_vkQueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, queueFamilies.data());

	for (int i = 0; i < queueFamilies.size(); i++)
	{
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physDevice, i, surface, &presentSupport);

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

i3D_vkSwapchainSupportInfo i3D_vkUtils::querySwapchainSupport(VkPhysicalDevice physDevice, VkSurfaceKHR surface)
{
	i3D_vkSwapchainSupportInfo info;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice, surface, &info.capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		info.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, info.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		info.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, info.presentModes.data());
	}

	return info;
}

// - Shader loading -----------------------------------------------------------------------------------

VkShaderModule i3D_vkUtils::createShaderModuleFromSPIRV(VkDevice device, const char* filePath)
{
	std::vector<char> spirvBytecode = i3D_fileUtils::getFileContents(filePath);
	
	if (spirvBytecode.empty())
	{
		i3D_logErrorMessage("VULKAN ERROR: Couldn't create the shader module from SPIR-V because the file at path \"%s\" couldn't be loaded.\n", filePath);
		return nullptr;
	}

	VkShaderModuleCreateInfo moduleInfo = {};
	moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.codeSize = spirvBytecode.size();
	moduleInfo.pCode = reinterpret_cast<const uint32_t*>(spirvBytecode.data());

	VkShaderModule shaderModule = nullptr;
	VkResult result = vkCreateShaderModule(device, &moduleInfo, nullptr, &shaderModule);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the shader module from SPIR-V. VkResult: %s\n", string_VkResult(result));
		return nullptr;
	}

	return shaderModule;
}

VkShaderModule i3D_vkUtils::createShaderModuleFromGLSL(VkDevice device, const char* filePath)
{
	/*
		Pick up from where you left off at https://youtu.be/Qbs9v1W7St8?t=416
	*/

	return nullptr;
}

// - Command buffers ----------------------------------------------------------------------------------

VkCommandBuffer i3D_vkUtils::beginSingleTimeCommands(VkDevice device, VkCommandPool pool)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = nullptr;

	VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to allocate the command buffer for the single-time commands. VkResult: %s\n", string_VkResult(result));
		return nullptr;
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	result = vkBeginCommandBuffer(commandBuffer, &beginInfo);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to record the command buffer for the single-time commands. VkResult: %s\n", string_VkResult(result));
		
		vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
		return nullptr;
	}

	return commandBuffer;
}

void i3D_vkUtils::endSingleTimeCommands(VkDevice device, VkCommandPool pool, VkQueue gfxQueue, VkCommandBuffer buffer)
{
	// TODO: Add error checking to all of these?
	vkEndCommandBuffer(buffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buffer;

	vkQueueSubmit(gfxQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(gfxQueue);

	vkFreeCommandBuffers(device, pool, 1, &buffer);
}

// - Buffers ------------------------------------------------------------------------------------------

bool i3D_vkUtils::findMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags memPropertyFlags, uint32_t& outputMemoryType)
{
	VkPhysicalDeviceMemoryProperties memProperties = {};
	vkGetPhysicalDeviceMemoryProperties(physDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && ((memProperties.memoryTypes[i].propertyFlags & memPropertyFlags) == memPropertyFlags))
		{
			outputMemoryType = i;
			return true;
		}
	}

	i3D_logErrorMessage("VULKAN ERROR: Failed to find a suitable memory type.\n");
	return false;
}

// - Formats ------------------------------------------------------------------------------------------

bool i3D_vkUtils::findSupportedFormat(VkPhysicalDevice physDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkFormat& outFormat)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props = {};
		vkGetPhysicalDeviceFormatProperties(physDevice, format, &props);

		if ((tiling == VK_IMAGE_TILING_LINEAR) && ((props.linearTilingFeatures & features) == features))
		{
			outFormat = format;
			return true;
		}
		else if ((tiling == VK_IMAGE_TILING_OPTIMAL) && ((props.optimalTilingFeatures & features) == features))
		{
			outFormat = format;
			return true;
		}
	}

	i3D_logErrorMessage("VULKAN ERROR: Failed to find a suitable format.\n");
	return false;
}

bool i3D_vkUtils::findDepthFormat(VkPhysicalDevice physDevice, VkFormat& outFormat)
{
	return findSupportedFormat
	(
		physDevice,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
		outFormat
	);
}

bool i3D_vkUtils::formatHasStencilComponent(VkFormat format)
{
	return (format == VK_FORMAT_D32_SFLOAT_S8_UINT) || (format == VK_FORMAT_D24_UNORM_S8_UINT);
}

// - Images -------------------------------------------------------------------------------------------

bool i3D_vkUtils::createImage(VkDevice device, VkPhysicalDevice physDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memProperties, VkImage& image, VkDeviceMemory& imageMem)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = numSamples;
	imageInfo.flags = 0;

	VkResult result = vkCreateImage(device, &imageInfo, nullptr, &image);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the image. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	VkMemoryRequirements memRequirements = {};
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	uint32_t memoryType = 0;

	if (!findMemoryType
	(
		physDevice,
		memRequirements.memoryTypeBits,
		memProperties,
		memoryType
	))
	{
		i3D_logErrorMessage("VULKAN ERROR: Couldn't find a suitable type for the image memory.\n");
		return false;
	}

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memoryType;

	result = vkAllocateMemory(device, &allocInfo, nullptr, &imageMem);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to allocate the image memory. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	vkBindImageMemory(device, image, imageMem, 0);
	return true;
}

VkImageView i3D_vkUtils::createImageView(VkDevice device, VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspect)
{
	VkImageView imageView = nullptr;

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.image = image;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspect;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &imageView);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the image view. VkResult: %s\n", string_VkResult(result));
		return nullptr;
	}

	return imageView;
}

bool i3D_vkUtils::copyBufferToImage(VkDevice device, VkCommandBuffer cmdBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
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
		cmdBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&copyRegion
	);

	return true;
}

bool i3D_vkUtils::transitionImageLayout(VkDevice device, VkCommandBuffer cmdBuffer, VkImage image, uint32_t mipLevels, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (formatHasStencilComponent(format))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkPipelineStageFlags srcStage, dstStage;

	if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if ((oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL))
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		i3D_logErrorMessage("VULKAN ERROR: Image layout transition \"%s -> %s\" is unsupported. Supported transitions include \"UNDEFINED -> TRANSFER_DST_OPTIMAL\", \"TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL\" and \"UNDEFINED -> DEPTH_STENCIL_ATTACHMENT_OPTIMAL\".\n", string_VkImageLayout(oldLayout), string_VkImageLayout(newLayout));
		return false;
	}

	vkCmdPipelineBarrier
	(
		cmdBuffer,
		srcStage,
		dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	return true;
}

bool i3D_vkUtils::generateMipmaps(VkDevice device, VkPhysicalDevice physDevice, VkCommandBuffer cmdBuffer, VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
	VkFormatProperties formatProperties = {};
	vkGetPhysicalDeviceFormatProperties(physDevice, format, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		i3D_logErrorMessage("VULKAN ERROR: Couldn't generate mipmaps because the image format does not support linear blitting.\n");
		return false;
	}

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier
		(
			cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier
		);

		VkImageBlit blit = {};

		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;

		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { (mipWidth > 1 ? mipWidth / 2 : 1), (mipHeight > 1 ? mipHeight / 2 : 1), 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage
		(
			cmdBuffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&blit,
			VK_FILTER_LINEAR
		);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier
		(
			cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier
		);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier
	(
		cmdBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&barrier
	);

	return true;
}