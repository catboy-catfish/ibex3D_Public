#include <ibexVulkan/vkTextureClass.h>
#include <ibexVulkan/vkUtils.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/image.h>

#include <algorithm>
#include <cmath>

bool vkTextureClass::initImageAndView(VkDevice device, VkPhysicalDevice physDevice, const char* imgFilePath, VkCommandPool cmdPool, VkQueue gfxQueue)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(imgFilePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (pixels == nullptr)
	{
		vkUtils::printVkError("vkTextureClass::initImageAndView()", "Couldn't load the texture data.\n");
		return false;
	}

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	VkBuffer stagingBuffer = nullptr;
	VkDeviceMemory stagingBufferMemory = nullptr;

	if (!vkUtils::createBuffer
	(
		physDevice,
		device,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	))
	{
		vkUtils::printVkError("vkTextureClass::initImageAndView()", "Couldn't create the staging buffer and allocate memory.\n");
		stbi_image_free(pixels);
		return false;
	}

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<uint32_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	stbi_image_free(pixels);

	if (!vkUtils::createImage
	(
		physDevice,
		device,
		texWidth,
		texHeight,
		mipLevels,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		image,
		imageMemory
	))
	{
		vkUtils::printVkError("vkTextureClass::initImageAndView()", "Couldn't create the image.\n");
		vkUtils::destroyBuffer(device, stagingBuffer, stagingBufferMemory);
		return false;
	}

	// TODO: Error check these three later?
	vkUtils::transitionImageLayout
	(
		device,
		image,
		mipLevels,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		cmdPool,
		gfxQueue
	);

	vkUtils::copyBufferToImage
	(
		device,
		cmdPool,
		gfxQueue,
		stagingBuffer,
		image,
		static_cast<uint32_t>(texWidth),
		static_cast<uint32_t>(texHeight)
	);

	vkUtils::generateMipmaps(device, physDevice, cmdPool, gfxQueue, image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
	vkUtils::destroyBuffer(device, stagingBuffer, stagingBufferMemory);

	// ----------------------------------------------------------------------------------------------------

	imageView = vkUtils::createImageView(device, image, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	if (imageView == nullptr)
	{
		vkUtils::printVkError("vkTextureClass::initImageView()", "Couldn't create the image view.\n");
		return false;
	}

	return true;
}

bool vkTextureClass::initSampler(VkDevice device, VkPhysicalDevice physDevice)
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
		vkUtils::printVkResultError(result, "vkRenderingContext::initTextureSampler()", "Couldn't create the texture sampler.");
		return false;
	}

	return true;
}

bool vkTextureClass::initialize(VkDevice device, VkPhysicalDevice physDevice, const char* imgFilePath, VkCommandPool cmdPool, VkQueue gfxQueue)
{
	if (!initImageAndView(device, physDevice, imgFilePath, cmdPool, gfxQueue))
	{
		return false;
	}

	if (!initSampler(device, physDevice))
	{
		return false;
	}
	
	return true;
}

void vkTextureClass::cleanup(VkDevice device)
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