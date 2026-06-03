#include <ibex3D/vulkan/textureObject.h>
#include <ibex3D/vulkan/bufferObject.h>
#include <ibex3D/vulkan/utils.h>

#include <ibex3D/utility/logger.h>

#define STB_IMAGE_IMPLEMENTATION
#include <thirdparty/stb/image.h>

#include <algorithm>
#include <cmath>

// ----------------------------------------------------------------------------------------------------

bool vkTextureObject::initImageAndView(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* imgFilePath)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(imgFilePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (pixels == nullptr)
	{
		logger::logError("vkTextureObject::initImageAndView(): An error occured while trying to load the texture data.", __FILE__, __LINE__ - 4);
		return false;
	}

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	vkBufferObject stagingBuffer;
	if (!stagingBuffer.initialize
	(
		device,
		physDevice,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	))
	{
		logger::logError("vkTextureObject::initImageAndView(): Couldn't create the staging buffer and allocate memory.", __FILE__, __LINE__ - 9);
		stbi_image_free(pixels);
		return false;
	}

	void* data;

	if (!stagingBuffer.mapBufferMemory(device, 0, imageSize, 0, &data))
	{
		logger::logError("vkTextureObject::initImageAndView(): Couldn't map the staging buffer memory.", __FILE__, __LINE__ - 2);
		return false;
	}
	
	memcpy(data, pixels, imageSize);

	stagingBuffer.unmapBufferMemory(device);
	stbi_image_free(pixels);

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (!vkUtils::createImage
	(
		device, physDevice,
		texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		image, imageMemory
	))
	{
		logger::logError("vkTextureObject::initImageAndView(): Couldn't create the image.", __FILE__, __LINE__ - 9);
		stagingBuffer.cleanup(device);
		return false;
	}

	// New: Recording the functions below into the same command buffer before ending it.
	// I'm wondering how much of a good idea this is???

	VkCommandBuffer cmdBuffer = vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (!vkUtils::transitionImageLayout
	(
		device, cmdBuffer,
		image, mipLevels, VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	))
	{
		logger::logError("vkTextureObject::initImageAndView(): Couldn't transition the image layout.", __FILE__, __LINE__ - 7);
		stagingBuffer.cleanup(device);
		return false;
	}

	// vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, cmdBuffer);
	// VkCommandBuffer cmdBuffer = vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (!vkUtils::copyBufferToImage
	(
		device, cmdBuffer,
		stagingBuffer.buffer, image,
		static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)
	))
	{
		logger::logError("vkTextureObject::initImageAndView(): Couldn't copy the staging buffer data to the image.", __FILE__, __LINE__ - 7);
		stagingBuffer.cleanup(device);
		return false;
	}

	// vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, cmdBuffer);
	// VkCommandBuffer cmdBuffer = vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (!vkUtils::generateMipmaps
	(
		device, physDevice, cmdBuffer,
		image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels
	))
	{
		logger::logError("vkTextureObject::initImageAndView(): Couldn't generate the image mipmaps.", __FILE__, __LINE__ - 6);
		stagingBuffer.cleanup(device);
		return false;
	}

	vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, cmdBuffer);
	stagingBuffer.cleanup(device);

	// ----------------------------------------------------------------------------------------------------

	imageView = vkUtils::createImageView(device, image, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	if (imageView == nullptr)
	{
		logger::logError("vkTextureObject::initImageAndView(): Couldn't create the image view.", __FILE__, __LINE__ - 4);
		return false;
	}

	return true;
}

bool vkTextureObject::initSampler(VkDevice device, VkPhysicalDevice physDevice)
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
		vkUtils::logErrorWithResult(result, "vkTextureObject::initImageAndView(): An error occurred while trying to create the image sampler.", __FILE__, __LINE__ - 4);
		return false;
	}

	return true;
}

bool vkTextureObject::initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* imgFilePath)
{
	if (!initImageAndView(device, physDevice, cmdPool, gfxQueue, imgFilePath))
	{
		return false;
	}

	if (!initSampler(device, physDevice))
	{
		return false;
	}
	
	return true;
}

void vkTextureObject::cleanup(VkDevice device)
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
}