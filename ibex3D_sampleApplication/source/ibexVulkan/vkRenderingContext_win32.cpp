#include <ibexVulkan/vkRenderingContext.h>
#include <ibexVulkan/vkMeshClass.h>
#include <ibexVulkan/vkUtils.h>

#include <ibex3D/utility/utilFunctions.h>
#include <ibex3D/utility/windowsUtils.h>

#include <vulkan/vulkan_win32.h>

#include <stdio.h>
#include <set>

static bool myGlobalBool = true;
static const int MAX_FRAMES_IN_FLIGHT = 2;

static std::vector<const char*> requiredExtensions;
static std::vector<const char*> requiredDeviceExtensions;

#ifdef IBEX3D_VULKAN_USE_VALIDATION_LAYERS
static std::vector<const char*> requiredLayers;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData
)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		printf("VULKAN ERROR (Validation layer) - %s\n", pCallbackData->pMessage);
	}
	
	return VK_FALSE;
}

static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info)
{
	info = {};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	info.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	info.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	info.pfnUserCallback = debugCallback;
	info.pUserData = nullptr;
}
#endif

// - Utility functions --------------------------------------------------------------------------------

static void configureRequiredExtensionsAndLayers()
{
	requiredExtensions.clear();
	requiredExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef IBEX3D_VULKAN_USE_VALIDATION_LAYERS
	requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	requiredLayers.clear();
	requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

	requiredDeviceExtensions.clear();
	requiredDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

// - Public functions ---------------------------------------------------------------------------------

bool vkRenderingContext::initialize(const char* appName, void* wndMemory)
{	
	int wndWidth, wndHeight;
	IBEX3D_BASSERT(win32Utils::getWindowDimensions(static_cast<HWND>(wndMemory), wndWidth, wndHeight));
	
	IBEX3D_BASSERT(initInstance(appName));
	IBEX3D_BASSERT(initSurface(wndMemory));
	IBEX3D_BASSERT(initPhysicalDevice());
	IBEX3D_BASSERT(initLogicalDevice());
	IBEX3D_BASSERT(initSwapchain(wndWidth, wndHeight));
	IBEX3D_BASSERT(initRenderPass());
	IBEX3D_BASSERT(initDescriptorSetLayout());
	IBEX3D_BASSERT(initGraphicsPipeline());
	IBEX3D_BASSERT(initFramebuffers());
	IBEX3D_BASSERT(initCommandPool());
	IBEX3D_BASSERT(initMeshClass());
	IBEX3D_BASSERT(initCommandBuffers());
	IBEX3D_BASSERT(initSyncObjects());

	return true;
}

void vkRenderingContext::setMeshRotation(float rotation)
{
	if (m_meshClass != nullptr)
	{
		m_meshClass->setMeshRotation(rotation);
	}
}

bool vkRenderingContext::drawFrame()
{	
	vkWaitForFences(m_logicalDevice, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(m_logicalDevice, m_swapchain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{	
		recreateSwapchain();
		return true;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::drawFrame()", "Couldn't acquire the swapchain image.");
		return false;
	}

	if (m_meshClass != nullptr)
	{
		m_meshClass->updateUniformBuffer(m_currentFrame, m_swapchainExtent);
	}

	// Only reset the fence if we are submitting work.
	vkResetFences(m_logicalDevice, 1, &m_inFlightFences[m_currentFrame]);

	vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);
	recordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);

	// - Submitting command buffer ------------------------------------------------------------------------

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

	result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::drawFrame()", "Couldn't submit the draw command buffer.");
		return false;
	}

	// - Presentation -------------------------------------------------------------------------------------

	VkSwapchainKHR swapchains[] = { m_swapchain };

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_wasJustResized)
	{
		recreateSwapchain();
		m_wasJustResized = false;
	}
	else if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::drawFrame()", "Couldn't enqueue the image for presentation.");
		return false;
	}

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	return true;
}

void vkRenderingContext::enableResizedFlag()
{
	m_wasJustResized = true;
}

void vkRenderingContext::cleanup()
{
	cleanupLogicalDevice();
	cleanupInstance();
}

// ----------------------------------------------------------------------------------------------------
// - Main functions -----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool vkRenderingContext::initInstance(const char* appName)
{
	configureRequiredExtensionsAndLayers();

#ifdef IBEX3D_VULKAN_USE_VALIDATION_LAYERS
	if (!checkInstanceLayerSupport())
	{
		vkUtils::printVkError("vkRenderingContext::initInstance()", "Requested validation layers unavailable on this device.");
		return false;
	}
#endif

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = appName;
	applicationInfo.pEngineName = "ibex3D";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &applicationInfo;
	instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());

#ifdef IBEX3D_VULKAN_USE_VALIDATION_LAYERS
	VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
	populateDebugMessengerCreateInfo(messengerInfo);

	instanceInfo.ppEnabledLayerNames = requiredLayers.data();
	instanceInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
	instanceInfo.pNext = static_cast<void*>(&messengerInfo);
#else
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.pNext = nullptr;
#endif

	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_instance);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::initInstance()", "Couldn't create the instance.");
		return false;
	}

#ifdef IBEX3D_VULKAN_USE_VALIDATION_LAYERS
	result = vkExtFunctions::CreateDebugUtilsMessengerEXT(m_instance, &messengerInfo, nullptr, &m_debugMessenger);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::initInstance()", "Couldn't create the debug utils messenger.");
		return false;
	}
#endif

	return true;
}

bool vkRenderingContext::initSurface(void* wndMemory)
{
	if (wndMemory == nullptr)
	{
		vkUtils::printVkError("vkRenderingContext::initSurface()", "Argument \"void* wndMemory\" is nullptr.");
		return false;
	}

	m_wndMemory = wndMemory;

	VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.hinstance = GetModuleHandleA(nullptr);
	surfaceInfo.hwnd = static_cast<HWND>(wndMemory);

	VkResult result = vkCreateWin32SurfaceKHR(m_instance, &surfaceInfo, nullptr, &m_surface);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::initSurface()", "Couldn't create the Win32 window surface.");
		return false;
	}
	
	return true;
}

bool vkRenderingContext::initPhysicalDevice()
{
	uint32_t numDevices = 0;
	vkEnumeratePhysicalDevices(m_instance, &numDevices, nullptr);

	if (numDevices == 0)
	{
		vkUtils::printVkError("vkRenderingContext::initPhysicalDevice()", "Couldn't find any GPU(s) with Vulkan support.");
		return false;
	}

	std::vector<VkPhysicalDevice> devices(numDevices);
	vkEnumeratePhysicalDevices(m_instance, &numDevices, devices.data());

	for (const auto& device : devices)
	{
		bool extensionsSupported = checkPhysicalDeviceExtensionSupport(device);
		
		if (vkUtils::checkPhysicalDeviceSuitability(device, m_surface, extensionsSupported))
		{
			m_physicalDevice = device;
			break;
		}
	}

	if (m_physicalDevice == nullptr)
	{
		vkUtils::printVkError("vkRenderingContext::initPhysicalDevice()", "Couldn't find any suitable GPU.");
		return false;
	}

	return true;
}

bool vkRenderingContext::initLogicalDevice()
{
	vkQueueFamilyIndices indices = vkUtils::findQueueFamilies(m_physicalDevice, m_surface);

	if (!indices.isComplete())
	{
		vkUtils::printVkError("vkRenderingContext::initLogicalDevice()", "One or more of the required queue families are missing.");
		return false;
	}

	std::vector<VkDeviceQueueCreateInfo> queueInfos;

	std::set<int> uniqueQueueFamilies =
	{
		indices.graphicsFamily,
		indices.presentFamily
	};

	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueInfo = {};

		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = queueFamily;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &queuePriority;

		queueInfos.push_back(queueInfo);
	}

	// This is empty for now, but we'll come back to it.
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo logicalDeviceInfo = {};
	logicalDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	logicalDeviceInfo.pEnabledFeatures = &deviceFeatures;
	logicalDeviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
	logicalDeviceInfo.pQueueCreateInfos = queueInfos.data();
	logicalDeviceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
	logicalDeviceInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();

#ifdef IBEX3D_VULKAN_USE_VALIDATION_LAYERS
	logicalDeviceInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
	logicalDeviceInfo.ppEnabledLayerNames = requiredLayers.data();
#else
	logicalDeviceInfo.enabledLayerCount = 0;
#endif

	VkResult result = vkCreateDevice(m_physicalDevice, &logicalDeviceInfo, nullptr, &m_logicalDevice);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::initLogicalDevice()", "Couldn't create the logical device.");
		return false;
	}

	vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_logicalDevice, indices.presentFamily, 0, &m_presentQueue);

	return true;
}

bool vkRenderingContext::initSwapchain(int wndWidth, int wndHeight)
{
#pragma region Swapchain
	vkSwapchainSupportInfo scSupport = vkUtils::querySwapchainSupport(m_physicalDevice, m_surface);

	VkSurfaceFormatKHR surfaceFormat = vkUtils::chooseSurfaceFormat(scSupport.formats);
	VkPresentModeKHR presentMode = vkUtils::choosePresentMode(scSupport.presentModes);
	VkExtent2D extent = vkUtils::chooseExtent(scSupport.capabilities, wndWidth, wndHeight);

	uint32_t imageCount = scSupport.capabilities.minImageCount + 1;

	if ((scSupport.capabilities.maxImageCount > 0) && (imageCount > scSupport.capabilities.maxImageCount))
	{
		imageCount = scSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainInfo = {};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = m_surface;
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = surfaceFormat.format;
	swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainInfo.imageExtent = extent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	vkQueueFamilyIndices indices = vkUtils::findQueueFamilies(m_physicalDevice, m_surface);

	if (!indices.isComplete())
	{
		vkUtils::printVkError("vkRenderingContext::initSwapchain()", "One or more of the required queue families are missing.");
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

	VkResult result = vkCreateSwapchainKHR(m_logicalDevice, &swapchainInfo, nullptr, &m_swapchain);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::initSwapchain()", "Couldn't create the swapchain.");
		return false;
	}

	result = vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &imageCount, nullptr);
	m_swapchainImages.resize(imageCount);
	result = vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &imageCount, m_swapchainImages.data());

	m_swapchainImageFormat = surfaceFormat.format;
	m_swapchainExtent = extent;
#pragma endregion

#pragma region Image views
	m_swapchainImageViews.resize(m_swapchainImages.size());

	for (size_t i = 0; i < m_swapchainImages.size(); i++)
	{
		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = m_swapchainImages[i];
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = m_swapchainImageFormat;
		imageViewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;

		result = vkCreateImageView(m_logicalDevice, &imageViewInfo, nullptr, &m_swapchainImageViews[i]);

		if (result != VK_SUCCESS)
		{
			vkUtils::printVkResultError(result, "vkRenderingContext::initSwapchain()", "Couldn't create one or more of the image views.");
			return false;
		}
	}
#pragma endregion

	return true;
}

bool vkRenderingContext::initRenderPass()
{
#pragma region Color attachment and reference
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
#pragma endregion

#pragma region Subpass and dependency
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;	// The index 0 represents our first subpass.
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
#pragma endregion

#pragma region Render pass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &subpassDependency;

	VkResult result = vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::initRenderPass()", "Couldn't create the render pass.");
		return false;
	}
#pragma endregion

	return true;
}

bool vkRenderingContext::initDescriptorSetLayout()
{
	m_meshClass = new vkMeshClass;

	if (!m_meshClass->initDescriptorSetLayout(m_logicalDevice))
	{
		return false;
	}
	
	return true;
}

bool vkRenderingContext::initGraphicsPipeline()
{
#pragma region Shader modules and stages info
	// TODO: Figure out how to compile the GLSL shaders into SPIR-V at runtime using libshaderc
	auto vtxShaderBytecode = ibex3D_utilFunctions::readFile("shaders/shader.vert.spv");
	if (vtxShaderBytecode.empty()) return false;

	auto frgShaderBytecode = ibex3D_utilFunctions::readFile("shaders/shader.frag.spv");
	if (frgShaderBytecode.empty()) return false;

	VkShaderModule vtxShaderModule = vkUtils::createShaderModule(m_logicalDevice, vtxShaderBytecode);

	if (vtxShaderModule == nullptr)
	{
		return false;
	}

	VkShaderModule frgShaderModule = vkUtils::createShaderModule(m_logicalDevice, frgShaderBytecode);

	if (frgShaderModule == nullptr)
	{
		if (vtxShaderModule != nullptr)
		{
			vkDestroyShaderModule(m_logicalDevice, vtxShaderModule, nullptr);
			vtxShaderModule = nullptr;
		}

		return false;
	}

	VkPipelineShaderStageCreateInfo vtxShaderStageInfo = {};
	vtxShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vtxShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vtxShaderStageInfo.module = vtxShaderModule;
	vtxShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo frgShaderStageInfo = {};
	frgShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frgShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frgShaderStageInfo.module = frgShaderModule;
	frgShaderStageInfo.pName = "main";
#pragma endregion

#pragma region Dynamic state info
	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicStateInfo.pDynamicStates = dynamicStates.data();
#pragma endregion

#pragma region Vertex input and assembly states
	auto bindingDesc = vkVertex::getBindingDesc();
	auto attribDescs = vkVertex::getAttributeDescs();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDescs.size());
	vertexInputInfo.pVertexAttributeDescriptions = attribDescs.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
#pragma endregion

#pragma region Viewport info
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_swapchainExtent.width);
	viewport.height = static_cast<float>(m_swapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportStateInfo = {};
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.pViewports = &viewport;
	viewportStateInfo.scissorCount = 1;
	viewportStateInfo.pScissors = &scissor;
#pragma endregion

#pragma region Rasterization state info
	VkPipelineRasterizationStateCreateInfo rasterStateInfo = {};
	rasterStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;	// The length of this thing holy fuck
	rasterStateInfo.depthClampEnable = VK_FALSE;
	rasterStateInfo.rasterizerDiscardEnable = false;
	rasterStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterStateInfo.lineWidth = 1.0f;
	rasterStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterStateInfo.depthBiasEnable = VK_FALSE;
	rasterStateInfo.depthBiasConstantFactor = 0.0f;
	rasterStateInfo.depthBiasClamp = 0.0f;
	rasterStateInfo.depthBiasSlopeFactor = 0.0f;
#pragma endregion

#pragma region Multisampling info
	// Disabled for now.
	VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.sampleShadingEnable = VK_FALSE;
	multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisamplingInfo.minSampleShading = 1.0f;
	multisamplingInfo.pSampleMask = nullptr;
	multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
	multisamplingInfo.alphaToOneEnable = VK_FALSE;
#pragma endregion

#pragma region Color blending info
	// Revisit this if you want to implement alpha blending!
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOpEnable = VK_FALSE;
	colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachment;
	colorBlendInfo.blendConstants[0] = 0.0f;
	colorBlendInfo.blendConstants[1] = 0.0f;
	colorBlendInfo.blendConstants[2] = 0.0f;
	colorBlendInfo.blendConstants[3] = 0.0f;
#pragma endregion

#pragma region Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_meshClass->descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VkResult result = vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::initGraphicsPipeline()", "Couldn't create the graphics pipeline layout.");
		
		if (frgShaderModule != nullptr)
		{
			vkDestroyShaderModule(m_logicalDevice, frgShaderModule, nullptr);
			frgShaderModule = nullptr;
		}

		if (vtxShaderModule != nullptr)
		{
			vkDestroyShaderModule(m_logicalDevice, vtxShaderModule, nullptr);
			vtxShaderModule = nullptr;
		}

		return false;
	}
#pragma endregion

#pragma region Graphics pipeline
	VkPipelineShaderStageCreateInfo shaderStages[] =
	{
		vtxShaderStageInfo,
		frgShaderStageInfo
	};
	
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pViewportState = &viewportStateInfo;
	pipelineInfo.pRasterizationState = &rasterStateInfo;
	pipelineInfo.pMultisampleState = &multisamplingInfo;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlendInfo;
	pipelineInfo.pDynamicState = &dynamicStateInfo;
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::initGraphicsPipeline()", "Couldn't create the graphics pipeline.");

		if (frgShaderModule != nullptr)
		{
			vkDestroyShaderModule(m_logicalDevice, frgShaderModule, nullptr);
			frgShaderModule = nullptr;
		}

		if (vtxShaderModule != nullptr)
		{
			vkDestroyShaderModule(m_logicalDevice, vtxShaderModule, nullptr);
			vtxShaderModule = nullptr;
		}

		return false;
	}
#pragma endregion

#pragma region Shader stage cleanup
	if (frgShaderModule != nullptr)
	{
		vkDestroyShaderModule(m_logicalDevice, frgShaderModule, nullptr);
		frgShaderModule = nullptr;
	}
	
	if (vtxShaderModule != nullptr)
	{
		vkDestroyShaderModule(m_logicalDevice, vtxShaderModule, nullptr);
		vtxShaderModule = nullptr;
	}
#pragma endregion
	return true;
}

bool vkRenderingContext::initFramebuffers()
{
	m_swapchainFramebuffers.resize(m_swapchainImageViews.size());

	for (size_t i = 0; i < m_swapchainImageViews.size(); i++)
	{
		VkImageView attachments[] = { m_swapchainImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_swapchainExtent.width;
		framebufferInfo.height = m_swapchainExtent.height;
		framebufferInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]);

		if (result != VK_SUCCESS)
		{
			vkUtils::printVkResultError(result, "vkRenderingContext::initFramebuffers()", "Couldn't create one or more of the required framebuffers.");
			return false;
		}
	}

	return true;
}

bool vkRenderingContext::initCommandPool()
{
	vkQueueFamilyIndices indices = vkUtils::findQueueFamilies(m_physicalDevice, m_surface);

	if (!indices.isComplete())
	{
		vkUtils::printVkError("vkRenderingContext::initCommandBuffers()", "One or more of the required queue families are missing.");
		return false;
	}

	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = indices.graphicsFamily;

	VkResult result = vkCreateCommandPool(m_logicalDevice, &commandPoolInfo, nullptr, &m_commandPool);;

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::initCommandBuffers()", "Couldn't create the command pool.");
		return false;
	}
	
	return true;
}

bool vkRenderingContext::initMeshClass()
{	
	if (m_meshClass != nullptr)
	{
		if (!m_meshClass->initialize(m_physicalDevice, m_logicalDevice, m_commandPool, m_graphicsQueue, MAX_FRAMES_IN_FLIGHT))
		{
			return false;
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool vkRenderingContext::initCommandBuffers()
{
	m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo commandBufferInfo = {};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.commandPool = m_commandPool;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

	VkResult result = vkAllocateCommandBuffers(m_logicalDevice, &commandBufferInfo, m_commandBuffers.data());

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::initCommandBuffers()", "Couldn't allocate the command buffers.");
		return false;
	}

	return true;
}

bool vkRenderingContext::initSyncObjects()
{
	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkResult result = vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]);

		if (result != VK_SUCCESS)
		{
			vkUtils::printVkResultError(result, "vkRenderingContext::initSyncObjects()", "Couldn't create the \"image available\" semaphore for one or more frames in flight.");
			return false;
		}

		result = vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]);

		if (result != VK_SUCCESS)
		{
			vkUtils::printVkResultError(result, "vkRenderingContext::initSyncObjects()", "Couldn't create the \"render finished\" semaphore for one or more frames in flight.");
			return false;
		}

		result = vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &m_inFlightFences[i]);

		if (result != VK_SUCCESS)
		{
			vkUtils::printVkResultError(result, "vkRenderingContext::initSyncObjects()", "Couldn't create the \"in flight\" fence for one or more frames in flight.");
			return false;
		}
	}

	return true;
}

bool vkRenderingContext::recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	VkResult result = vkBeginCommandBuffer(buffer, &beginInfo);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::recordCommandBuffer()", "Couldn't begin recording the command buffer.");
		return false;
	}

	VkClearValue clearColor = { {{ 0.0f, 0.0f, 0.0f, 1.0f }} };

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapchainExtent;
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_swapchainExtent.width);
	viewport.height = static_cast<float>(m_swapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(buffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchainExtent;
	vkCmdSetScissor(buffer, 0, 1, &scissor);

	m_meshClass->draw(buffer, m_pipelineLayout, m_currentFrame);

	vkCmdEndRenderPass(buffer);

	result = vkEndCommandBuffer(buffer);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::recordCommandBuffer()", "Couldn't finish recording the command buffer.");
		return false;
	}

	return true;
}

void vkRenderingContext::recreateSwapchain()
{
	int wndWidth, wndHeight;

	if (!win32Utils::getWindowDimensions(static_cast<HWND>(m_wndMemory), wndWidth, wndHeight))
	{
		return;
	}

	// Idk how it does it, but this prevents a validation layer error.
	vkDeviceWaitIdle(m_logicalDevice);

	cleanupSwapchain(m_logicalDevice);

	initSwapchain(wndWidth, wndHeight);
	initFramebuffers();
}

void vkRenderingContext::cleanupSwapchain(VkDevice device)
{
	for (auto framebuffer : m_swapchainFramebuffers)
	{
		if (framebuffer != nullptr)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
	}

	m_swapchainFramebuffers.clear();

	for (auto imageView : m_swapchainImageViews)
	{
		if (imageView != nullptr)
		{
			vkDestroyImageView(device, imageView, nullptr);
		}
	}

	m_swapchainImageViews.clear();

	if (m_swapchain != nullptr)
	{
		vkDestroySwapchainKHR(device, m_swapchain, nullptr);
		m_swapchain = nullptr;
	}
}

void vkRenderingContext::cleanupLogicalDevice()
{
	if (m_logicalDevice != nullptr)
	{
		vkDeviceWaitIdle(m_logicalDevice);

		cleanupSwapchain(m_logicalDevice);

		if (m_meshClass != nullptr)
		{
			m_meshClass->cleanup(m_logicalDevice);
			delete m_meshClass;
			m_meshClass = nullptr;
		}

		if (m_graphicsPipeline != nullptr)
		{
			vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
			m_graphicsPipeline = nullptr;
		}

		if (m_pipelineLayout != nullptr)
		{
			vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
			m_pipelineLayout = nullptr;
		}

		if (m_renderPass != nullptr)
		{
			vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
			m_renderPass = nullptr;
		}

		for (auto& semaphore : m_imageAvailableSemaphores)
		{
			vkDestroySemaphore(m_logicalDevice, semaphore, nullptr);
			semaphore = nullptr;
		}

		m_imageAvailableSemaphores.clear();

		for (auto& semaphore : m_renderFinishedSemaphores)
		{
			vkDestroySemaphore(m_logicalDevice, semaphore, nullptr);
			semaphore = nullptr;
		}

		m_renderFinishedSemaphores.clear();

		for (auto& fence : m_inFlightFences)
		{
			vkDestroyFence(m_logicalDevice, fence, nullptr);
			fence = nullptr;
		}

		m_inFlightFences.clear();

		if (m_commandPool != nullptr)
		{
			vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
			m_commandPool = nullptr;
		}

		vkDestroyDevice(m_logicalDevice, nullptr);
		m_logicalDevice = nullptr;
	}
}

void vkRenderingContext::cleanupInstance()
{
	if (m_instance != nullptr)
	{
#ifdef IBEX3D_VULKAN_USE_VALIDATION_LAYERS
		if (m_debugMessenger != nullptr)
		{
			vkExtFunctions::DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
			m_debugMessenger = nullptr;
		}
#endif
		if (m_surface != nullptr)
		{
			vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
			m_surface = nullptr;
		}

		vkDestroyInstance(m_instance, nullptr);
		m_instance = nullptr;

		m_wndMemory = nullptr;
	}
}

// ----------------------------------------------------------------------------------------------------
// - Helper functions ---------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

bool vkRenderingContext::checkInstanceLayerSupport()
{
#ifdef IBEX3D_VULKAN_USE_VALIDATION_LAYERS
	if (requiredLayers.empty()) return true;

	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : requiredLayers)
	{
		bool layerFound = false;
		
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}
#endif

	return true;
}

bool vkRenderingContext::checkPhysicalDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> extensions
	(
		requiredDeviceExtensions.begin(),
		requiredDeviceExtensions.end()
	);

	for (const auto& extension : availableExtensions)
	{
		extensions.erase(extension.extensionName);
	}

	return extensions.empty();
}

void vkRenderingContext::printAvailableInstanceExtensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	printf("Available instance extensions:\n");

	for (const auto& extension : extensions)
	{
		printf("- %s\n", extension.extensionName);
	}
}

void vkRenderingContext::printAvailableInstanceLayers()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> layers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

	printf("Available instance layers:\n");

	for (const auto& layer : layers)
	{
		printf("- %s\n", layer.layerName);
	}
}