#include <ibex3D/vulkan/vkRenderingContext.h>
#include <ibex3D/vulkan/vkUtils.h>

#include <ibex3D/core/win32.h>
#include <vulkan/vulkan_win32.h>

#include <ibex3D/utility/miscellaneous.h>

#include <stdio.h>
#include <set>
#include <map>

static const int MAX_FRAMES_IN_FLIGHT = 2;

static std::vector<const char*> instExtensionNames =
{
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_KHR_SURFACE_EXTENSION_NAME
};

static std::vector<const char*> deviceExtensionNames =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static std::vector<const char*> instLayerNames;

// - Public functions ---------------------------------------------------------------------------------

bool vkRenderingContext::initialize(void* wndMemory)
{		
	int wndWidth, wndHeight;
	IBEX3D_BASSERT(win32Utils::getWindowDimensions(static_cast<HWND>(wndMemory), wndWidth, wndHeight));

	IBEX3D_BASSERT(initInstance());
	IBEX3D_BASSERT(initSurface(wndMemory));
	IBEX3D_BASSERT(initPhysicalDevice(VK_SAMPLE_COUNT_1_BIT));
	IBEX3D_BASSERT(initLogicalDevice());
	IBEX3D_BASSERT(initSwapchain(wndWidth, wndHeight));
	IBEX3D_BASSERT(initRenderPass());
	IBEX3D_BASSERT(initDescriptorSetLayout());
	IBEX3D_BASSERT(initGraphicsPipeline());
	IBEX3D_BASSERT(initCommandPool());
	IBEX3D_BASSERT(initColorResources());
	IBEX3D_BASSERT(initDepthResources());
	IBEX3D_BASSERT(initFramebuffers());
	IBEX3D_BASSERT(initModel());
	IBEX3D_BASSERT(initCommandBuffers());
	IBEX3D_BASSERT(initSyncObjects());

	return true;
}

void vkRenderingContext::setMeshRotation(float rotation)
{
	m_meshClass.setMeshRotation(rotation);
}

bool vkRenderingContext::drawFrame()
{	
	vkWaitForFences(m_logicalDevice, 1, &m_frameFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(m_logicalDevice, m_swapchain, UINT64_MAX, m_frameSemaphores[m_currentFrame], nullptr, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{	
		IBEX3D_BASSERT(recreateSwapchain());
		return true;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::drawFrame()", "Couldn't acquire the swapchain image.");
		return false;
	}

	m_meshClass.updateUniformBuffer(m_currentFrame, m_swapchainExtent);

	vkResetFences(m_logicalDevice, 1, &m_frameFences[m_currentFrame]);
	vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);
	recordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);

	// - Submitting command buffer ------------------------------------------------------------------------

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.commandBufferCount = 1;
	submitInfo.pWaitSemaphores = &m_frameSemaphores[m_currentFrame];
	submitInfo.pSignalSemaphores = &m_swapchainSemaphores[imageIndex];
	submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];
	submitInfo.pWaitDstStageMask = &waitStage;

	result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_frameFences[m_currentFrame]);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::drawFrame()", "Couldn't submit the draw command to the command buffer.");
		return false;
	}

	// - Presentation -------------------------------------------------------------------------------------

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.swapchainCount = 1;
	presentInfo.pWaitSemaphores = &m_swapchainSemaphores[imageIndex];
	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_wasJustResized)
	{
		IBEX3D_BASSERT(recreateSwapchain());
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

bool vkRenderingContext::initInstance()
{
	configureRequiredExtensions();

#ifdef IBEX3D_VULKAN_VALIDATION
	if (!checkInstanceLayerSupport())
	{
		vkUtils::printVkError("vkRenderingContext::initInstance()", "Requested validation layers unavailable on this device.");
		return false;
	}
#endif

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "No name";
	applicationInfo.pEngineName = "ibex3D";
	applicationInfo.apiVersion = VK_API_VERSION_1_3;
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &applicationInfo;
	instanceInfo.ppEnabledExtensionNames = instExtensionNames.data();
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instExtensionNames.size());

#ifdef IBEX3D_VULKAN_VALIDATION
	VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
	vkUtils::populateDebugMessengerCreateInfo(messengerInfo);

	instanceInfo.ppEnabledLayerNames = instLayerNames.data();
	instanceInfo.enabledLayerCount = static_cast<uint32_t>(instLayerNames.size());
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

#ifdef IBEX3D_VULKAN_VALIDATION
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

bool vkRenderingContext::initPhysicalDevice(VkSampleCountFlagBits msaaSamplesUsed)
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

	std::multimap<int, VkPhysicalDevice> candidates;

	for (const auto& device : devices)
	{
		bool extensionsSupported = checkPhysicalDeviceExtensionSupport(device);
		
		int score = vkUtils::ratePhysicalDeviceSuitability(device, m_surface, extensionsSupported);
		candidates.insert(std::make_pair(score, device));
	}

	if (candidates.rbegin()->first > 0)
	{
		m_physicalDevice = candidates.rbegin()->second;

		auto maxMsaaSamples = vkUtils::getMaxUsableSampleCount(m_physicalDevice);
		m_msaaSamples = (msaaSamplesUsed > maxMsaaSamples) ? maxMsaaSamples : msaaSamplesUsed;
	}
	else
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

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;

	VkDeviceCreateInfo logicalDeviceInfo = {};
	logicalDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	logicalDeviceInfo.pEnabledFeatures = &deviceFeatures;
	logicalDeviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
	logicalDeviceInfo.pQueueCreateInfos = queueInfos.data();
	logicalDeviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionNames.size());
	logicalDeviceInfo.ppEnabledExtensionNames = deviceExtensionNames.data();

#ifdef IBEX3D_VULKAN_VALIDATION
	logicalDeviceInfo.enabledLayerCount = static_cast<uint32_t>(instLayerNames.size());
	logicalDeviceInfo.ppEnabledLayerNames = instLayerNames.data();
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
	vkSwapchainSupportInfo scSupport = vkUtils::querySwapchainSupport(m_physicalDevice, m_surface);

	VkSurfaceFormatKHR surfaceFormat = vkUtils::chooseSurfaceFormat(scSupport.formats);
	VkPresentModeKHR presentMode = vkUtils::choosePresentMode(scSupport.presentModes, m_useVsync);
	VkExtent2D extent = vkUtils::chooseExtent(scSupport.capabilities, wndWidth, wndHeight);

	m_swapchainImageCount = scSupport.capabilities.minImageCount + 1;

	if ((scSupport.capabilities.maxImageCount > 0) && (m_swapchainImageCount > scSupport.capabilities.maxImageCount))
	{
		m_swapchainImageCount = scSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainInfo = {};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = m_surface;
	swapchainInfo.minImageCount = m_swapchainImageCount;
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

	result = vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &m_swapchainImageCount, nullptr);
	
	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::initSwapchain()", "Couldn't get the number of swapchain images.");
		return false;
	}

	m_swapchainImages.resize(m_swapchainImageCount);
	
	result = vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &m_swapchainImageCount, m_swapchainImages.data());

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::initSwapchain()", "Couldn't retrieve the images from the swapchain.");
		return false;
	}

	m_swapchainImageFormat = surfaceFormat.format;
	m_swapchainExtent = extent;

	// ----------------------------------------------------------------------------------------------------

	m_swapchainImageViews.resize(m_swapchainImages.size());

	for (size_t i = 0; i < m_swapchainImages.size(); i++)
	{
		m_swapchainImageViews[i] = vkUtils::createImageView(m_logicalDevice, m_swapchainImages[i], 1, m_swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		if (m_swapchainImageViews[i] == nullptr)
		{
			vkUtils::printVkError("vkRenderingContext::initSwapchain()", "Couldn't create one or more of the swapchain image views.");
			return false;
		}
	}

	return true;
}

bool vkRenderingContext::initRenderPass()
{
#pragma region Color attachment and reference
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_swapchainImageFormat;
	colorAttachment.samples = m_msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
#pragma endregion

#pragma region Depth attachment and reference
	VkAttachmentDescription depthAttachment = {};

	bool success = false;
	depthAttachment.format = vkUtils::findDepthFormat(m_physicalDevice, success);

	if (!success)
	{
		vkUtils::printVkError("vkRenderingContext::initRenderPass()", "Couldn't find a suitable depth attachment format.");
		return false;
	}

	depthAttachment.samples = m_msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
#pragma endregion

#pragma region Color resolve attachment and reference
	VkAttachmentDescription colorAttachmentResolve = {};
	colorAttachmentResolve.format = m_swapchainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentResolveRef = {};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
#pragma endregion

#pragma region Subpass and dependency
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;	// The index 0 represents our first subpass.
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
#pragma endregion

#pragma region Render pass
	std::array<VkAttachmentDescription, 3> attachments =
	{
		colorAttachment,
		depthAttachment,
		colorAttachmentResolve
	};
	
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &subpassDependency;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();

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
	if (!m_meshClass.initDescriptorSetLayout(m_logicalDevice))
	{
		return false;
	}
	
	return true;
}

bool vkRenderingContext::initGraphicsPipeline()
{
#pragma region Shader modules and stages info
	// TODO: Figure out how to compile the GLSL shaders into SPIR-V at runtime using libshaderc
	auto vtxShaderBytecode = ibex3D_utilFunctions::readFile("assets/shaders/shader.vert.spv");
	if (vtxShaderBytecode.empty()) return false;

	auto frgShaderBytecode = ibex3D_utilFunctions::readFile("assets/shaders/shader.frag.spv");
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
	VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.rasterizationSamples = m_msaaSamples;
	multisamplingInfo.sampleShadingEnable = VK_TRUE;
	multisamplingInfo.minSampleShading = 0.2f;
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

#pragma region Depth stencil state info
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
	depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.depthTestEnable = VK_TRUE;
	depthStencilInfo.depthWriteEnable = VK_TRUE;
	depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.minDepthBounds = 0.0f;		// Optional
	depthStencilInfo.maxDepthBounds = 1.0f;		// Optional
	depthStencilInfo.stencilTestEnable = VK_FALSE;
	depthStencilInfo.front = {};				// Optional
	depthStencilInfo.back = {};					// Optional
#pragma endregion

#pragma region Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_meshClass.descriptorSetLayout;
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
	pipelineInfo.pDepthStencilState = &depthStencilInfo;
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

bool vkRenderingContext::initColorResources()
{	
	VkFormat colorFormat = m_swapchainImageFormat;

	if (!vkUtils::createImage
	(
		m_logicalDevice,
		m_physicalDevice,
		m_swapchainExtent.width,
		m_swapchainExtent.height,
		1,
		m_msaaSamples,
		colorFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_colorImage,
		m_colorImageMemory
	))
	{
		vkUtils::printVkError("vkRenderingContext::initColorResources()", "Couldn't create the color image.");
		return false;
	}

	m_colorImageView = vkUtils::createImageView
	(
		m_logicalDevice,
		m_colorImage,
		1,
		colorFormat,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	if (m_colorImageView == nullptr)
	{
		vkUtils::printVkError("vkRenderingContext::initColorResources()", "Couldn't create the color image view.");
		return false;
	}

	return true;
}

bool vkRenderingContext::initDepthResources()
{
	bool success = false;
	VkFormat depthFormat = vkUtils::findDepthFormat(m_physicalDevice, success);

	if (!success)
	{
		vkUtils::printVkError("vkRenderingContext::initDepthResources()", "Couldn't find a suitable format for the depth image.");
		return false;
	}

	if (!vkUtils::createImage
	(
		m_logicalDevice,
		m_physicalDevice,
		m_swapchainExtent.width,
		m_swapchainExtent.height,
		1,
		m_msaaSamples,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_depthImage,
		m_depthImageMemory
	))
	{
		vkUtils::printVkError("vkRenderingContext::initDepthResources()", "Couldn't create the depth image.");
		return false;
	}

	m_depthImageView = vkUtils::createImageView(m_logicalDevice, m_depthImage, 1, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	if (m_depthImageView == nullptr)
	{
		vkUtils::printVkError("vkRenderingContext::initDepthResources()", "Couldn't create the depth image view.");
		return false;
	}

	if (!vkUtils::transitionImageLayout
	(
		m_logicalDevice,
		m_commandPool,
		m_graphicsQueue,
		m_depthImage,
		1,
		depthFormat,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	))
	{
		vkUtils::printVkError("vkRenderingContext::initDepthResources()", "Couldn't transition the depth image layout.");
		return false;
	}

	return true;
}

bool vkRenderingContext::initFramebuffers()
{
	m_swapchainFramebuffers.resize(m_swapchainImageViews.size());

	for (size_t i = 0; i < m_swapchainImageViews.size(); i++)
	{
		std::array<VkImageView, 3> attachments =
		{
			m_colorImageView,
			m_depthImageView,
			m_swapchainImageViews[i]
		};
		
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
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

bool vkRenderingContext::initModel()
{	
	if (!m_textureClass.initialize(m_logicalDevice, m_physicalDevice, "assets/images/viking_room.png", m_commandPool, m_graphicsQueue))
	{
		return false;
	}
	
	if (!m_meshClass.initialize
	(
		m_logicalDevice,
		m_physicalDevice, 
		m_commandPool,
		m_graphicsQueue,
		MAX_FRAMES_IN_FLIGHT,
		"assets/models/viking_room.obj",
		&m_textureClass
	))
	{
		return false;
	}

	return true;
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
	m_swapchainSemaphores.resize(m_swapchainImageCount);
	m_frameSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_frameFences.resize(MAX_FRAMES_IN_FLIGHT);
	
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < m_swapchainImageCount; i++)
	{
		VkResult result = vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_swapchainSemaphores[i]);

		if (result != VK_SUCCESS)
		{
			vkUtils::printVkResultError(result, "vkRenderingContext::initSyncObjects()", "Couldn't create the semaphore for one or more swapchain images.");
			return false;
		}
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkResult result = vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_frameSemaphores[i]);

		if (result != VK_SUCCESS)
		{
			vkUtils::printVkResultError(result, "vkRenderingContext::initSyncObjects()", "Couldn't create the semaphore for one or more frames in flight.");
			return false;
		}

		result = vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &m_frameFences[i]);

		if (result != VK_SUCCESS)
		{
			vkUtils::printVkResultError(result, "vkRenderingContext::initSyncObjects()", "Couldn't create the fence for one or more frames in flight.");
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

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapchainExtent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

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

	m_meshClass.draw(buffer, m_pipelineLayout, m_currentFrame);

	vkCmdEndRenderPass(buffer);

	result = vkEndCommandBuffer(buffer);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkRenderingContext::recordCommandBuffer()", "Couldn't finish recording the command buffer.");
		return false;
	}

	return true;
}

bool vkRenderingContext::recreateSwapchain()
{
	int wndWidth = 0;
	int wndHeight = 0;
	
	if (!win32Utils::getWindowDimensions(static_cast<HWND>(m_wndMemory), wndWidth, wndHeight))
	{
		return false;
	}

	if (wndWidth == 0 || wndHeight == 0)
	{
		// Window is minimized
		return true;
	}

	vkDeviceWaitIdle(m_logicalDevice);
	cleanupSwapchain(m_logicalDevice);

	if (!initSwapchain(wndWidth, wndHeight))
	{
		vkUtils::printVkError("vkRenderingContext::recreateSwapchain()", "Couldn't recreate the swapchain.");
		return false;
	}
	
	if (!initColorResources())
	{
		vkUtils::printVkError("vkRenderingContext::recreateSwapchain()", "Couldn't recreate the color buffer resources.");
		return false;
	}

	if (!initDepthResources())
	{
		vkUtils::printVkError("vkRenderingContext::recreateSwapchain()", "Couldn't recreate the depth buffer resources.");
		return false;
	}

	if (!initFramebuffers())
	{
		vkUtils::printVkError("vkRenderingContext::recreateSwapchain()", "Couldn't recreate the framebuffers.");
		return false;
	}

	return true;
}

void vkRenderingContext::cleanupSwapchain(VkDevice device)
{
	if (m_depthImageView != nullptr)
	{
		vkDestroyImageView(m_logicalDevice, m_depthImageView, nullptr);
		m_depthImageView = nullptr;
	}
	
	if (m_depthImage != nullptr)
	{
		vkDestroyImage(m_logicalDevice, m_depthImage, nullptr);
		m_depthImage = nullptr;
	}

	if (m_depthImageMemory != nullptr)
	{
		vkFreeMemory(m_logicalDevice, m_depthImageMemory, nullptr);
		m_depthImageMemory = nullptr;
	}

	// ----------------------------------------------------------------------------------------------------

	if (m_colorImageView != nullptr)
	{
		vkDestroyImageView(m_logicalDevice, m_colorImageView, nullptr);
		m_colorImageView = nullptr;
	}

	if (m_colorImage != nullptr)
	{
		vkDestroyImage(m_logicalDevice, m_colorImage, nullptr);
		m_colorImage = nullptr;
	}

	if (m_colorImageMemory != nullptr)
	{
		vkFreeMemory(m_logicalDevice, m_colorImageMemory, nullptr);
		m_colorImageMemory = nullptr;
	}

	// ----------------------------------------------------------------------------------------------------

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

		m_meshClass.cleanup(m_logicalDevice);
		m_textureClass.cleanup(m_logicalDevice);

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

		for (auto& semaphore : m_swapchainSemaphores)
		{
			vkDestroySemaphore(m_logicalDevice, semaphore, nullptr);
			semaphore = nullptr;
		}

		m_swapchainSemaphores.clear();

		for (auto& semaphore : m_frameSemaphores)
		{
			vkDestroySemaphore(m_logicalDevice, semaphore, nullptr);
			semaphore = nullptr;
		}

		m_frameSemaphores.clear();

		for (auto& fence : m_frameFences)
		{
			vkDestroyFence(m_logicalDevice, fence, nullptr);
			fence = nullptr;
		}

		m_frameFences.clear();

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
#ifdef IBEX3D_VULKAN_VALIDATION
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

void vkRenderingContext::configureRequiredExtensions()
{
#ifdef IBEX3D_VULKAN_VALIDATION
	instExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	instLayerNames.push_back("VK_LAYER_KHRONOS_validation");
#endif
}

bool vkRenderingContext::checkInstanceLayerSupport()
{
	if (instLayerNames.empty()) return true;

#ifdef IBEX3D_VULKAN_VALIDATION
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : instLayerNames)
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
		deviceExtensionNames.begin(),
		deviceExtensionNames.end()
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