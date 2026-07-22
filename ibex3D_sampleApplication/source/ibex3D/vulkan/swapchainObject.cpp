#include <ibex3D/vulkan/swapchainObject.h>
#include <ibex3D/vulkan/utils.h>

#include <array>
#include <stdio.h>

#include <vulkan/vk_enum_string_helper.h>

// ----------------------------------------------------------------------------------------------------

bool i3D_vkSwapchainObject::initSwapchain(VkDevice device, VkPhysicalDevice physDevice, VkSurfaceKHR surface, int wndWidth, int wndHeight, bool vSync)
{
	i3D_vkSwapchainSupportInfo scSupport = i3D_vkUtils::querySwapchainSupport(physDevice, surface);

	VkSurfaceFormatKHR format = i3D_vkUtils::chooseSurfaceFormat(scSupport.formats);
	VkPresentModeKHR presentMode = i3D_vkUtils::choosePresentMode(scSupport.presentModes, vSync);
	VkExtent2D extent = i3D_vkUtils::chooseExtent(scSupport.capabilities, wndWidth, wndHeight);

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

	i3D_vkQueueFamilyIndices indices = i3D_vkUtils::findQueueFamilies(physDevice, surface);

	if (!indices.isComplete())
	{
		fprintf(stderr, "VULKAN ERROR: Couldn't create the swapchain because one or more of the requred queue families are missing.\n");
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
		fprintf(stderr, "VULKAN ERROR: Failed to create the swapchain. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	result = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);

	if (result != VK_SUCCESS)
	{
		fprintf(stderr, "VULKAN ERROR: Failed to retrieve the swapchain image count. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	swapchainImages.resize(imageCount);

	result = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

	if (result != VK_SUCCESS)
	{
		fprintf(stderr, "VULKAN ERROR: Failed to retrieve the swapchain images. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	imageFormat = format.format;
	imageExtent = extent;

	// ----------------------------------------------------------------------------------------------------

	swapchainImageViews.resize(imageCount);

	for (size_t i = 0; i < imageCount; i++)
	{
		swapchainImageViews[i] = i3D_vkUtils::createImageView(device, swapchainImages[i], 1, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		if (swapchainImageViews[i] == nullptr)
		{
			fprintf(stderr, "VULKAN ERROR: Failed to create one of the swapchain image views.\n");
			return false;
		}
	}

	return true;
}

bool i3D_vkSwapchainObject::initColorResources(VkDevice device, VkPhysicalDevice physDevice, VkSampleCountFlagBits msaaSamples)
{
	VkFormat colorFormat = imageFormat;

	if (!i3D_vkUtils::createImage
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
		fprintf(stderr, "VULKAN ERROR: Failed to create the swapchain color image.\n");
		return false;
	}

	colorImageView = i3D_vkUtils::createImageView
	(
		device,
		colorImage,
		1,
		colorFormat,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	if (colorImageView == nullptr)
	{
		fprintf(stderr, "VULKAN ERROR: Failed to create the swapchain color image view.\n");
		return false;
	}

	return true;
}

bool i3D_vkSwapchainObject::initDepthResources(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, VkSampleCountFlagBits msaaSamples)
{
	VkFormat depthFormat;

	if (!i3D_vkUtils::findDepthFormat(physDevice, depthFormat))
	{
		fprintf(stderr, "VULKAN ERROR: Couldn't find a suitable format for the swapchain depth image.\n");
		return false;
	}

	if (!i3D_vkUtils::createImage
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
		fprintf(stderr, "VULKAN ERROR: Failed to create the swapchain depth image.\n");
		return false;
	}

	depthImageView = i3D_vkUtils::createImageView(device, depthImage, 1, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	if (depthImageView == nullptr)
	{
		fprintf(stderr, "VULKAN ERROR: Failed to create the swapchain depth image view.\n");
		return false;
	}

	VkCommandBuffer cmdBuffer = i3D_vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (!i3D_vkUtils::transitionImageLayout
	(
		device,
		cmdBuffer,
		depthImage,
		1,
		depthFormat,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	))
	{
		fprintf(stderr, "VULKAN ERROR: Failed to transition the swapchain depth image layout.\n");
		return false;
	}

	i3D_vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, cmdBuffer);

	return true;
}

void i3D_vkSwapchainObject::cleanupSwapchain(VkDevice device)
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

void i3D_vkSwapchainObject::cleanupColorResources(VkDevice device)
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

void i3D_vkSwapchainObject::cleanupDepthResources(VkDevice device)
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

float i3D_vkSwapchainObject::getAspectRatio()
{
	return imageExtent.width / static_cast<float>(imageExtent.height);
}
