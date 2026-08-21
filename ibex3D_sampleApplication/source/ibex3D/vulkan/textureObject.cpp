#include <ibex3D/vulkan/textureObject.h>
#include <ibex3D/vulkan/bufferObject.h>
#include <ibex3D/vulkan/utils.h>

#include <ibex3D/core/logger.h>

#include <algorithm>
#include <cmath>

#include <vulkan/vk_enum_string_helper.h>

#define STB_IMAGE_IMPLEMENTATION
#include <thirdparty/stb/image.h>

// ----------------------------------------------------------------------------------------------------

bool i3D_vkTextureObject::initImage(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* imgFilePath)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(imgFilePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (pixels == nullptr)
	{
		i3D_logErrorMessage("VULKAN ERROR: Couldn't load the data from the image file at path \"%s\". Have you ensured that the path to the image file is correct?\n", imgFilePath);
		return false;
	}

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	i3D_vkBufferObject stagingBuffer;
	if (!stagingBuffer.initialize
	(
		device,
		physDevice,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	))
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to initialize the staging buffer for the texture object.\n");
		stbi_image_free(pixels);
		return false;
	}

	void* data;

	if (!stagingBuffer.mapBufferMemory(device, 0, imageSize, 0, &data))
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to map the staging buffer memory.\n");
		return false;
	}
	
	memcpy(data, pixels, imageSize);

	stagingBuffer.unmapBufferMemory(device);
	stbi_image_free(pixels);

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (!i3D_vkUtils::createImage
	(
		device, physDevice,
		texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		image, imageMemory
	))
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the texture image.\n");
		stagingBuffer.cleanup(device);
		return false;
	}

	// Recording the functions below into the same command buffer before ending it.
	// I'm wondering how much of a good idea this is???

	VkCommandBuffer cmdBuffer = i3D_vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (!i3D_vkUtils::transitionImageLayout
	(
		device, cmdBuffer,
		image, mipLevels, VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	))
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to transition the texture image layout.\n");
		stagingBuffer.cleanup(device);
		return false;
	}

	// i3D_vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, cmdBuffer);
	// cmdBuffer = i3D_vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (!i3D_vkUtils::copyBufferToImage
	(
		device, cmdBuffer,
		stagingBuffer.buffer, image,
		static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)
	))
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to copy the staging buffer data to the texture image.\n");
		stagingBuffer.cleanup(device);
		return false;
	}

	// i3D_vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, cmdBuffer);
	// cmdBuffer = i3D_vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (!i3D_vkUtils::generateMipmaps
	(
		device, physDevice, cmdBuffer,
		image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels
	))
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to generate the mipmaps for the texture image.\n");
		stagingBuffer.cleanup(device);
		return false;
	}

	i3D_vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, cmdBuffer);
	stagingBuffer.cleanup(device);

	return true;
}

bool i3D_vkTextureObject::initImage_VMA(VkDevice device, VkPhysicalDevice physDevice, VmaAllocator allocator, VkCommandPool cmdPool, VkQueue gfxQueue, const char* imgFilePath)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(imgFilePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (pixels == nullptr)
	{
		i3D_logErrorMessage("VULKAN ERROR: Couldn't load the data from the image file at path \"%s\". Have you ensured that the path to the image file is correct?\n", imgFilePath);
		return false;
	}

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	i3D_vkBufferObject stagingBuffer;
	if (!stagingBuffer.initialize
	(
		device,
		physDevice,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	))
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to initialize the staging buffer for the texture object.\n");
		stbi_image_free(pixels);
		return false;
	}

	void* data;

	if (!stagingBuffer.mapBufferMemory(device, 0, imageSize, 0, &data))
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to map the staging buffer memory.\n");
		return false;
	}

	memcpy(data, pixels, imageSize);

	stagingBuffer.unmapBufferMemory(device);
	stbi_image_free(pixels);

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkImageCreateInfo imageCI = {};
	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.extent.width = texWidth;
	imageCI.extent.height = texHeight;
	imageCI.extent.depth = 1;
	imageCI.mipLevels = mipLevels;
	imageCI.arrayLayers = 1;
	imageCI.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCI.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.flags = 0;

	VmaAllocationCreateInfo imageAI = {};
	imageAI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	imageAI.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	VkResult result = vmaCreateImage(allocator, &imageCI, &imageAI, &image, &allocation, nullptr);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the texture image. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	// Recording the functions below into the same command buffer before ending it.
	// I'm wondering how much of a good idea this is???

	VkCommandBuffer cmdBuffer = i3D_vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (!i3D_vkUtils::transitionImageLayout
	(
		device, cmdBuffer,
		image, mipLevels, VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	))
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to transition the texture image layout.\n");
		stagingBuffer.cleanup(device);
		return false;
	}

	// i3D_vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, cmdBuffer);
	// cmdBuffer = i3D_vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (!i3D_vkUtils::copyBufferToImage
	(
		device, cmdBuffer,
		stagingBuffer.buffer, image,
		static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)
	))
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to copy the staging buffer data to the texture image.\n");
		stagingBuffer.cleanup(device);
		return false;
	}

	// i3D_vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, cmdBuffer);
	// cmdBuffer = i3D_vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (!i3D_vkUtils::generateMipmaps
	(
		device, physDevice, cmdBuffer,
		image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels
	))
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to generate the mipmaps for the texture image.\n");
		stagingBuffer.cleanup(device);
		return false;
	}

	i3D_vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, cmdBuffer);
	stagingBuffer.cleanup(device);
	
	return true;
}

bool i3D_vkTextureObject::initImageView(VkDevice device)
{
	imageView = i3D_vkUtils::createImageView(device, image, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	if (imageView == nullptr)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the texture image view.\n");
		return false;
	}

	return true;
}

bool i3D_vkTextureObject::initSampler(VkDevice device, VkPhysicalDevice physDevice)
{
	VkPhysicalDeviceProperties pdProperties = {};
	vkGetPhysicalDeviceProperties(physDevice, &pdProperties);

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = pdProperties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
	samplerInfo.minLod = 0.0f;

	VkResult result = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the texture image sampler. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	return true;
}

bool i3D_vkTextureObject::initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* imgFilePath)
{
	if (!initImage(device, physDevice, cmdPool, gfxQueue, imgFilePath))
	{
		return false;
	}

	if (!initImageView(device))
	{
		return false;
	}

	if (!initSampler(device, physDevice))
	{
		return false;
	}
	
	return true;
}

bool i3D_vkTextureObject::initialize_VMA(VkDevice device, VkPhysicalDevice physDevice, VmaAllocator allocator, VkCommandPool cmdPool, VkQueue gfxQueue, const char* imgFilePath)
{
	if (!initImage_VMA(device, physDevice, allocator, cmdPool, gfxQueue, imgFilePath))
	{
		return false;
	}

	if (!initImageView(device))
	{
		return false;
	}

	if (!initSampler(device, physDevice))
	{
		return false;
	}

	return true;
}

void i3D_vkTextureObject::cleanup(VkDevice device, VmaAllocator allocator)
{
	if (sampler != nullptr)
	{
		vkDestroySampler(device, sampler, nullptr);
		sampler = nullptr;
	}

	if (imageView != nullptr)
	{
		vkDestroyImageView(device, imageView, nullptr);
		imageView = nullptr;
	}

	if (imageMemory != nullptr)
	{
		vkFreeMemory(device, imageMemory, nullptr);
		imageMemory = nullptr;
	}

	if (image != nullptr)
	{
		vkDestroyImage(device, image, nullptr);
		image = nullptr;
	}

	if (allocation != nullptr)
	{
		vmaFreeMemory(allocator, allocation);
		allocation = nullptr;
	}
}