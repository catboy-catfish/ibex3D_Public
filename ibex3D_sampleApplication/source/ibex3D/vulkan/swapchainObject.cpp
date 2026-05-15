#include <ibex3D/vulkan/swapchainObject.h>
#include <ibex3D/vulkan/utils.h>

#include <ibex3D/utility/logger.h>

#include <array>

// ----------------------------------------------------------------------------------------------------

bool vkSwapchainObject::initSwapchain(VkDevice device, VkPhysicalDevice physDevice, VkSurfaceKHR surface, int wndWidth, int wndHeight, bool vSync)
{
	vkSwapchainSupportInfo scSupport = vkUtils::querySwapchainSupport(physDevice, surface);

	VkSurfaceFormatKHR format = vkUtils::chooseSurfaceFormat(scSupport.formats);
	VkPresentModeKHR presentMode = vkUtils::choosePresentMode(scSupport.presentModes, vSync);
	VkExtent2D extent = vkUtils::chooseExtent(scSupport.capabilities, wndWidth, wndHeight);

	imageCount = scSupport.capabilities.minImageCount + 1;

	if ((scSupport.capabilities.maxImageCount > 0) && (imageCount > scSupport.capabilities.maxImageCount))
	{
		imageCount = scSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainInfo = {};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = surface;
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = format.format;
	swapchainInfo.imageColorSpace = format.colorSpace;
	swapchainInfo.imageExtent = extent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	vkQueueFamilyIndices indices = vkUtils::findQueueFamilies(physDevice, surface);

	if (!indices.isComplete())
	{
		logger::logError("vkSwapchainObject::initSwapchain(): Couldn't create the swapchain because one or more of the required queue families are missing.", __FILE__, __LINE__ - 4);
		return false;
	}

	uint32_t queueFamilyIndices[] =
	{
		static_cast<uint32_t>(indices.graphicsFamily),
		static_cast<uint32_t>(indices.presentFamily),
	};

	if (indices.graphicsFamily != indices.presentFamily)
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.queueFamilyIndexCount = 0;
		swapchainInfo.pQueueFamilyIndices = nullptr;
	}

	swapchainInfo.preTransform = scSupport.capabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain);

	if (result != VK_SUCCESS)
	{
		vkUtils::logErrorWithResult(result, "vkSwapchainObject::initSwapchain(): An error occured while trying to create the swapchain.", __FILE__, __LINE__ - 4);
		return false;
	}

	result = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);

	if (result != VK_SUCCESS)
	{
		vkUtils::logErrorWithResult(result, "vkSwapchainObject::initSwapchain(): An error occured while trying to retrieve the number of swapchain images.", __FILE__, __LINE__ - 4);
		return false;
	}

	swapchainImages.resize(imageCount);

	result = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

	if (result != VK_SUCCESS)
	{
		vkUtils::logErrorWithResult(result, "vkSwapchainObject::initSwapchain(): An error occured while trying to retrieve the swapchain images.", __FILE__, __LINE__ - 4);
		return false;
	}

	imageFormat = format.format;
	imageExtent = extent;

	// ----------------------------------------------------------------------------------------------------

	swapchainImageViews.resize(imageCount);

	for (size_t i = 0; i < imageCount; i++)
	{
		swapchainImageViews[i] = vkUtils::createImageView(device, swapchainImages[i], 1, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		if (swapchainImageViews[i] == nullptr)
		{
			logger::logError("vkSwapchainObject::initSwapchain(): Couldn't create one or more of the swapchain image views.", __FILE__, __LINE__ - 4);
			return false;
		}
	}

	return true;
}

bool vkSwapchainObject::initColorResources(VkDevice device, VkPhysicalDevice physDevice, VkSampleCountFlagBits msaaSamples)
{
	VkFormat colorFormat = imageFormat;

	if (!vkUtils::createImage
	(
		device,
		physDevice,
		imageExtent.width,
		imageExtent.height,
		1,
		msaaSamples,
		colorFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		colorImage,
		colorImageMemory
	))
	{
		logger::logError("vkSwapchainObject::initColorResources(): Couldn't create the color image.", __FILE__, __LINE__ - 16);
		return false;
	}

	colorImageView = vkUtils::createImageView
	(
		device,
		colorImage,
		1,
		colorFormat,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	if (colorImageView == nullptr)
	{
		logger::logError("vkSwapchainObject::initColorResources(): Couldn't create the color image view.", __FILE__, __LINE__ - 11);
		return false;
	}

	return true;
}

bool vkSwapchainObject::initDepthResources(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, VkSampleCountFlagBits msaaSamples)
{
	VkFormat depthFormat;

	if (!vkUtils::findDepthFormat(physDevice, depthFormat))
	{
		logger::logError("vkSwapchainObject::initDepthResources(): Couldn't find a suitable format for the depth image.", __FILE__, __LINE__ - 2);
		return false;
	}

	if (!vkUtils::createImage
	(
		device,
		physDevice,
		imageExtent.width,
		imageExtent.height,
		1,
		msaaSamples,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthImage,
		depthImageMemory
	))
	{
		logger::logError("vkSwapchainObject::initDepthResources(): Couldn't create the depth image.", __FILE__, __LINE__ - 16);
		return false;
	}

	depthImageView = vkUtils::createImageView(device, depthImage, 1, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	if (depthImageView == nullptr)
	{
		logger::logError("vkSwapchainObject::initDepthResources(): Couldn't create the depth image view.", __FILE__, __LINE__ - 2);
		return false;
	}

	if (!vkUtils::transitionImageLayout
	(
		device,
		cmdPool,
		gfxQueue,
		depthImage,
		1,
		depthFormat,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	))
	{
		logger::logError("vkSwapchainObject::initDepthResources(): Couldn't transition the depth image layout.", __FILE__, __LINE__ - 12);
		return false;
	}

	return true;
}

void vkSwapchainObject::cleanupSwapchain(VkDevice device)
{
	for (auto imageView : swapchainImageViews)
	{
		if (imageView != nullptr)
		{
			vkDestroyImageView(device, imageView, nullptr);
		}
	}

	swapchainImageViews.clear();

	if (swapchain != nullptr)
	{
		vkDestroySwapchainKHR(device, swapchain, nullptr);
		swapchain = nullptr;
	}
}

void vkSwapchainObject::cleanupColorResources(VkDevice device)
{
	if (colorImageView != nullptr)
	{
		vkDestroyImageView(device, colorImageView, nullptr);
		colorImageView = nullptr;
	}

	if (colorImage != nullptr)
	{
		vkDestroyImage(device, colorImage, nullptr);
		colorImage = nullptr;
	}

	if (colorImageMemory != nullptr)
	{
		vkFreeMemory(device, colorImageMemory, nullptr);
		colorImageMemory = nullptr;
	}
}

void vkSwapchainObject::cleanupDepthResources(VkDevice device)
{
	if (depthImageView != nullptr)
	{
		vkDestroyImageView(device, depthImageView, nullptr);
		depthImageView = nullptr;
	}

	if (depthImage != nullptr)
	{
		vkDestroyImage(device, depthImage, nullptr);
		depthImage = nullptr;
	}

	if (depthImageMemory != nullptr)
	{
		vkFreeMemory(device, depthImageMemory, nullptr);
		depthImageMemory = nullptr;
	}
}

float vkSwapchainObject::getAspectRatio()
{
	return imageExtent.width / static_cast<float>(imageExtent.height);
}
