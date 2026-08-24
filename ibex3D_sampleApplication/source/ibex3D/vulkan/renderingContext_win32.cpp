#include <ibex3D/vulkan/renderingContext.h>
#include <ibex3D/vulkan/validation.h>
#include <ibex3D/vulkan/gfxPipelineBuilder.h>
#include <ibex3D/vulkan/utils.h>

#include <ibex3D/core/logger.h>
#include <ibex3D/core/fileAccess.h>
#include <ibex3D/core/windowsUtils.h>

#include <map>
#include <set>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <thirdparty/glm/ext/matrix_transform.hpp>
#include <thirdparty/glm/ext/matrix_clip_space.hpp>

#include <vulkan/vulkan_win32.h>
#include <vulkan/vk_enum_string_helper.h>

// ----------------------------------------------------------------------------------------------------

#define MAX_FRAMES_IN_FLIGHT 2

#define I3D_BASSERT(condition)	\
if (!condition)					\
{								\
	return false;				\
}								\

// ----------------------------------------------------------------------------------------------------

struct vkUniformBufferData
{
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	glm::vec3 cameraPosition;
	float padding;
};

// ----------------------------------------------------------------------------------------------------

static std::vector<const char*> getRequiredInstanceExtensions()
{
	std::vector<const char*> requiredExtensions =
	{
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef I3D_VULKAN_VALIDATION
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
	};

	return requiredExtensions;
}

static std::vector<const char*> getRequiredInstanceLayers()
{
#ifdef I3D_VULKAN_VALIDATION
	std::vector<const char*> layers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	return layers;
#else
	return std::vector<const char*>();
#endif
}

static std::vector<const char*> getRequiredDeviceExtensions()
{
	std::vector<const char*> requiredExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	return requiredExtensions;
}

static bool checkInstanceLayerSupport()
{
	auto instanceLayers = getRequiredInstanceLayers();

	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : instanceLayers)
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

	return true;
}

static bool checkPhysDeviceExtensionSupport(VkPhysicalDevice physDevice)
{
	auto deviceExtensions = getRequiredDeviceExtensions();

	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> extensions
	(
		deviceExtensions.begin(),
		deviceExtensions.end()
	);

	for (const auto& extension : availableExtensions)
	{
		extensions.erase(extension.extensionName);
	}

	return extensions.empty();
}

static int ratePhysicalDeviceSuitability(VkPhysicalDevice physDevice, VkSurfaceKHR surface)
{
	if (!checkPhysDeviceExtensionSupport(physDevice))
	{
		i3D_logErrorMessage("VULKAN ERROR: This GPU is unsuitable because it doesn't support the required Vulkan extensions.\n");
		return 0;
	}

	int score = 0;

	VkPhysicalDeviceProperties2 deviceProperties = {};
	deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

	vkGetPhysicalDeviceProperties2(physDevice, &deviceProperties);

	if (deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}

	score += deviceProperties.properties.limits.maxImageDimension2D;

	VkPhysicalDeviceFeatures deviceFeatures = {};
	vkGetPhysicalDeviceFeatures(physDevice, &deviceFeatures);

	if (!deviceFeatures.geometryShader)
	{
		i3D_logErrorMessage("VULKAN ERROR: This GPU is unsuitable because it doesn't support geometry shaders.\n");
		return 0;
	}

	if (!deviceFeatures.samplerAnisotropy)
	{
		i3D_logErrorMessage("VULKAN ERROR: This GPU is unsuitable because it doesn't support anisotropic texture filtering.\n");
		return 0;
	}

	i3D_vkSwapchainSupportInfo info = i3D_vkUtils::querySwapchainSupport(physDevice, surface);

	if ((info.formats.empty() || info.presentModes.empty()))
	{
		i3D_logErrorMessage("VULKAN ERROR: This GPU is unsuitable because it doesn't support the required swapchain formats or presentation modes.\n");
		return 0;
	}

	i3D_vkQueueFamilyIndices indices = i3D_vkUtils::findQueueFamilies(physDevice, surface);

	if (!indices.isComplete())
	{
		i3D_logErrorMessage("VULKAN ERROR: This GPU is unsuitable because one or more required queue families are missing.\n");
		return 0;
	}

	return score;
}

static VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physDevice)
{
	VkPhysicalDeviceProperties pdProperties = {};
	vkGetPhysicalDeviceProperties(physDevice, &pdProperties);

	VkSampleCountFlags counts =
		pdProperties.limits.framebufferColorSampleCounts &
		pdProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
	return VK_SAMPLE_COUNT_1_BIT;
}

static glm::mat4 getCameraViewMatrix(glm::vec3 position, glm::vec3 rotation)
{
	glm::mat4 viewMatrix = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 0.0f, 1.0f));	// Roll
	viewMatrix = glm::rotate(viewMatrix, rotation.z, glm::vec3(1.0f, 0.0f, 0.0f));					// Pitch
	viewMatrix = glm::rotate(viewMatrix, rotation.x, glm::vec3(0.0f, 1.0f, 0.0f));					// Yaw
	
	viewMatrix = glm::translate(viewMatrix, -position);

	return viewMatrix;
}

static glm::mat4 getCameraProjMatrix(float fovRadians, float aspectRatio)
{
	glm::mat4 projMatrix = glm::perspective(fovRadians, aspectRatio, 0.1f, 10.0f);
	projMatrix[1][1] *= -1.0f;
	
	return projMatrix;
}

// ----------------------------------------------------------------------------------------------------

bool i3D_vkRenderingContext::initialize(void* wndMemory)
{
	LONG wndWidth, wndHeight;
	I3D_BASSERT(i3D_windowsUtils::getWindowDimensions(static_cast<HWND>(wndMemory), wndWidth, wndHeight));

	I3D_BASSERT(initInstance());
	I3D_BASSERT(initSurface(wndMemory));
	I3D_BASSERT(initPhysicalDevice());
	I3D_BASSERT(initLogicalDevice());
	I3D_BASSERT(initVMA());
	I3D_BASSERT(initSwapchain(wndWidth, wndHeight));
	I3D_BASSERT(initRenderPass());
	I3D_BASSERT(initDescriptorSetLayouts());
	I3D_BASSERT(initGraphicsPipeline());
	I3D_BASSERT(initGraphicsPipeline2());
	I3D_BASSERT(initCommands());
	I3D_BASSERT(initSwapchainResources());
	I3D_BASSERT(initFramebuffers());
	I3D_BASSERT(initModelAndTexture());
	I3D_BASSERT(initUniformBuffers());
	I3D_BASSERT(initDescriptors());
	I3D_BASSERT(initSyncObjects());
	
	return true;
}

bool i3D_vkRenderingContext::drawFrame(float meshRotation)
{	
	m_currentMeshRotation = meshRotation;
	
	vkWaitForFences(m_device, 1, &m_frameFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain.swapchain, UINT64_MAX, m_frameSemaphores[m_currentFrame], nullptr, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{	
		return recreateSwapchain();
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to acquire the swapchain image. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	vkResetFences(m_device, 1, &m_frameFences[m_currentFrame]);
	vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);

	updateUniformBuffer(m_currentFrame);
	recordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);

	// ----------------------------------------------------------------------------------------------------

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
		i3D_logErrorMessage("VULKAN ERROR: Failed to submit the rendering commands to the command buffer. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	// ----------------------------------------------------------------------------------------------------

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.swapchainCount = 1;
	presentInfo.pWaitSemaphores = &m_swapchainSemaphores[imageIndex];
	presentInfo.pSwapchains = &m_swapchain.swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_refreshSwapchain)
	{
		I3D_BASSERT(recreateSwapchain());
		m_refreshSwapchain = false;
	}
	else if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to enqueue the image for presentation. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	return true;
}

void i3D_vkRenderingContext::refresh()
{
	m_refreshSwapchain = true;
}

void i3D_vkRenderingContext::cleanup()
{
	cleanupLogicalDevice();
	cleanupInstance();
}

// ----------------------------------------------------------------------------------------------------

bool i3D_vkRenderingContext::initInstance()
{
#ifdef I3D_VULKAN_VALIDATION
	if (!checkInstanceLayerSupport())
	{
		i3D_logErrorMessage("VULKAN ERROR: I3D_VULKAN_VALIDATION is enabled, but the required validation layers for this are unavailable on this device.\n");
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

	auto instanceExtensions = getRequiredInstanceExtensions();

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &applicationInfo;
	instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());

#ifdef I3D_VULKAN_VALIDATION
	auto instanceLayers = getRequiredInstanceLayers();
	instanceInfo.ppEnabledLayerNames = instanceLayers.data();
	instanceInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
	
	auto messengerInfo = i3D_vkValidation::debugMessengerCreateInfo();
	instanceInfo.pNext = &messengerInfo;
#endif

	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_instance);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the Vulkan instance. VkResult: %s\n", string_VkResult(result));
		return false;
	}

#ifdef I3D_VULKAN_VALIDATION
	result = i3D_vkValidation::createDebugMessenger(m_instance, &messengerInfo, nullptr, &m_debugMessenger);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the debug messenger. VkResult: %s\n", string_VkResult(result));
		return false;
	}
#endif

	return true;
}

bool i3D_vkRenderingContext::initSurface(void* wndMemory)
{
	if (wndMemory == nullptr)
	{
		i3D_logErrorMessage("VULKAN ERROR: Couldn't create the Vulkan surface because argument \"void* wndMemory\" is nullptr. You must pass the application window memory address (this would be an HWND on Windows) in order to use this function.\n");
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
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the Vulkan Win32 window surface. VkResult: %s\n", string_VkResult(result));
		return false;
	}
	
	return true;
}

bool i3D_vkRenderingContext::initPhysicalDevice()
{
	VkSampleCountFlagBits targetMsaaSamples = VK_SAMPLE_COUNT_4_BIT;
	
	uint32_t numDevices = 0;
	vkEnumeratePhysicalDevices(m_instance, &numDevices, nullptr);

	if (numDevices == 0)
	{
		i3D_logErrorMessage("VULKAN ERROR: Couldn't find any GPUs with Vulkan support.\n");
		return false;
	}

	std::vector<VkPhysicalDevice> devices(numDevices);
	vkEnumeratePhysicalDevices(m_instance, &numDevices, devices.data());

	std::multimap<int, VkPhysicalDevice> candidates;

	for (const auto& device : devices)
	{
		int score = ratePhysicalDeviceSuitability(device, m_surface);
		candidates.insert(std::make_pair(score, device));
	}

	if (candidates.rbegin()->first > 0)
	{
		m_physDevice = candidates.rbegin()->second;

		auto maxMsaaSamples = getMaxUsableSampleCount(m_physDevice);
		m_msaaSamples = (targetMsaaSamples > maxMsaaSamples) ? maxMsaaSamples : targetMsaaSamples;
	}
	else
	{
		i3D_logErrorMessage("VULKAN ERROR: Couldn't find any suitable GPU to use.\n");
		return false;
	}

	return true;
}

bool i3D_vkRenderingContext::initLogicalDevice()
{
	i3D_vkQueueFamilyIndices indices = i3D_vkUtils::findQueueFamilies(m_physDevice, m_surface);

	if (!indices.isComplete())
	{
		i3D_logErrorMessage("VULKAN ERROR: Couldn't create the logical device because one or more of the required queue families are missing.\n");
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

	auto deviceExtensions = getRequiredDeviceExtensions();
	logicalDeviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	logicalDeviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifdef I3D_VULKAN_VALIDATION
	auto instanceLayers = getRequiredInstanceLayers();
	logicalDeviceInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
	logicalDeviceInfo.ppEnabledLayerNames = instanceLayers.data();
#else
	logicalDeviceInfo.enabledLayerCount = 0;
#endif

	VkResult result = vkCreateDevice(m_physDevice, &logicalDeviceInfo, nullptr, &m_device);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the logical device. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);

	return true;
}

bool i3D_vkRenderingContext::initVMA()
{
	VmaAllocatorCreateInfo allocatorCI = {};
	allocatorCI.instance = m_instance;
	allocatorCI.physicalDevice = m_physDevice;
	allocatorCI.device = m_device;

	VkResult result = vmaCreateAllocator(&allocatorCI, &m_allocator);
	
	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the Vulkan memory allocator. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	return true;
}

bool i3D_vkRenderingContext::initSwapchain(int wndWidth, int wndHeight)
{	
	if (!m_swapchain.initSwapchain(m_device, m_physDevice, m_surface, wndWidth, wndHeight, true))
	{
		return false;
	}
	
	m_aspectRatio = m_swapchain.getAspectRatio();

	return true;
}

bool i3D_vkRenderingContext::initRenderPass()
{
	std::array<VkAttachmentDescription, 3> attachments = { {} };

	// Color attachment
	attachments[0].format = m_swapchain.imageFormat;
	attachments[0].samples = m_msaaSamples;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	// Depth attachment
	attachments[1].samples = m_msaaSamples;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
	if (!i3D_vkUtils::findDepthFormat(m_physDevice, attachments[1].format))
	{
		i3D_logErrorMessage("VULKAN ERROR: Couldn't find a suitable format for the render pass depth attachment.\n");
		return false;
	}

	// Resolve attachment
	attachments[2].format = m_swapchain.imageFormat;
	attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef = {};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0; // 0 represents the first subpass
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &subpassDependency;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();

	VkResult result = vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the render pass. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	return true;
}

bool i3D_vkRenderingContext::initDescriptorSetLayouts()
{
	i3D_vkDescriptorLayoutBuilder builder;

	// shader.vert: layout (binding = 0) uniform UniformBufferObject{} ubo;
	// shader.frag: layout (binding = 1) uniform sampler2D texSampler;
	builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
	builder.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);

	m_descriptorSetLayout = builder.buildLayout(m_device, 0, nullptr);

	if (m_descriptorSetLayout == nullptr)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the first descriptor set layout.\n");
		return false;
	}
	
	return true;
}

bool i3D_vkRenderingContext::initGraphicsPipeline()
{		
	i3D_vkGfxPipelineBuilder pipelineBuilder;
	pipelineBuilder.clearEverything();

	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	pipelineBuilder.initDynamicState(static_cast<uint32_t>(dynamicStates.size()), dynamicStates.data());

	auto bindingDesc = i3D_vkVertex::getBindingDesc();
	auto attribDescs = i3D_vkVertex::getAttributeDescs();

	pipelineBuilder.initVertexInputState(1, &bindingDesc, static_cast<uint32_t>(attribDescs.size()), attribDescs.data());
	pipelineBuilder.initInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.initViewportState(m_swapchain.imageExtent);
	pipelineBuilder.initRasterState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	pipelineBuilder.initMultisampleState_enabled(m_msaaSamples, VK_TRUE, VK_FALSE, VK_FALSE);
	pipelineBuilder.initColorBlendState_disabled();
	pipelineBuilder.initDepthStencilState_enabled(VK_TRUE, VK_COMPARE_OP_LESS);

	m_pipelineLayout = pipelineBuilder.buildPipelineLayout(m_device, 1, &m_descriptorSetLayout, 0, nullptr);

	if (m_pipelineLayout == nullptr)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the graphics pipeline layout.\n");
		return false;
	}

	VkShaderModule vtxShaderModule = i3D_vkUtils::createShaderModuleFromSPIRV(m_device, "assets/shaders/shader_vert.spv");

	if (vtxShaderModule == nullptr)
	{
		return false;
	}

	VkShaderModule frgShaderModule = i3D_vkUtils::createShaderModuleFromSPIRV(m_device, "assets/shaders/shader_frag.spv");

	if (frgShaderModule == nullptr)
	{
		if (vtxShaderModule != nullptr)
		{
			vkDestroyShaderModule(m_device, vtxShaderModule, nullptr);
			vtxShaderModule = nullptr;
		}

		return false;
	}

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = { {} };

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vtxShaderModule;
	shaderStages[0].pName = "main";
	
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = frgShaderModule;
	shaderStages[1].pName = "main";
	
	m_graphicsPipeline = pipelineBuilder.buildGraphicsPipeline(m_device, static_cast<uint32_t>(shaderStages.size()), shaderStages.data(), m_pipelineLayout, m_renderPass);

	if (m_graphicsPipeline == nullptr)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the graphics pipeline.\n");

		if (frgShaderModule != nullptr)
		{
			vkDestroyShaderModule(m_device, frgShaderModule, nullptr);
			frgShaderModule = nullptr;
		}

		if (vtxShaderModule != nullptr)
		{
			vkDestroyShaderModule(m_device, vtxShaderModule, nullptr);
			vtxShaderModule = nullptr;
		}

		return false;
	}

	if (frgShaderModule != nullptr)
	{
		vkDestroyShaderModule(m_device, frgShaderModule, nullptr);
		frgShaderModule = nullptr;
	}
	
	if (vtxShaderModule != nullptr)
	{
		vkDestroyShaderModule(m_device, vtxShaderModule, nullptr);
		vtxShaderModule = nullptr;
	}

	return true;
}

bool i3D_vkRenderingContext::initGraphicsPipeline2()
{
	i3D_vkGfxPipelineBuilder pipelineBuilder;
	pipelineBuilder.clearEverything();

	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	pipelineBuilder.initDynamicState(static_cast<uint32_t>(dynamicStates.size()), dynamicStates.data());

	auto bindingDesc = i3D_vkVertex::getBindingDesc();
	auto attribDescs = i3D_vkVertex::getAttributeDescs();

	pipelineBuilder.initVertexInputState(1, &bindingDesc, static_cast<uint32_t>(attribDescs.size()), attribDescs.data());
	pipelineBuilder.initInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.initViewportState(m_swapchain.imageExtent);
	pipelineBuilder.initRasterState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	pipelineBuilder.initMultisampleState_enabled(m_msaaSamples, VK_TRUE, VK_FALSE, VK_FALSE);
	pipelineBuilder.initColorBlendState_disabled();
	pipelineBuilder.initDepthStencilState_enabled(VK_TRUE, VK_COMPARE_OP_LESS);

	m_pipelineLayout2 = pipelineBuilder.buildPipelineLayout(m_device, 1, &m_descriptorSetLayout, 0, nullptr);

	if (m_pipelineLayout2 == nullptr)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the graphics pipeline layout.\n");
		return false;
	}

	VkShaderModule vtxShaderModule = i3D_vkUtils::createShaderModuleFromSPIRV(m_device, "assets/shaders/shader2_vert.spv");

	if (vtxShaderModule == nullptr)
	{
		return false;
	}

	VkShaderModule frgShaderModule = i3D_vkUtils::createShaderModuleFromSPIRV(m_device, "assets/shaders/shader2_frag.spv");

	if (frgShaderModule == nullptr)
	{
		if (vtxShaderModule != nullptr)
		{
			vkDestroyShaderModule(m_device, vtxShaderModule, nullptr);
			vtxShaderModule = nullptr;
		}

		return false;
	}

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = { {} };

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vtxShaderModule;
	shaderStages[0].pName = "main";

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = frgShaderModule;
	shaderStages[1].pName = "main";

	m_graphicsPipeline2 = pipelineBuilder.buildGraphicsPipeline(m_device, static_cast<uint32_t>(shaderStages.size()), shaderStages.data(), m_pipelineLayout2, m_renderPass);

	if (m_graphicsPipeline2 == nullptr)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the graphics pipeline.\n");

		if (frgShaderModule != nullptr)
		{
			vkDestroyShaderModule(m_device, frgShaderModule, nullptr);
			frgShaderModule = nullptr;
		}

		if (vtxShaderModule != nullptr)
		{
			vkDestroyShaderModule(m_device, vtxShaderModule, nullptr);
			vtxShaderModule = nullptr;
		}

		return false;
	}

	if (frgShaderModule != nullptr)
	{
		vkDestroyShaderModule(m_device, frgShaderModule, nullptr);
		frgShaderModule = nullptr;
	}

	if (vtxShaderModule != nullptr)
	{
		vkDestroyShaderModule(m_device, vtxShaderModule, nullptr);
		vtxShaderModule = nullptr;
	}

	return true;
}

bool i3D_vkRenderingContext::initCommands()
{
	i3D_vkQueueFamilyIndices indices = i3D_vkUtils::findQueueFamilies(m_physDevice, m_surface);

	if (!indices.isComplete())
	{
		i3D_logErrorMessage("VULKAN ERROR: Couldn't create the command pool because one or more required queue families are missing.\n");
		return false;
	}

	m_commandPools.resize(MAX_FRAMES_IN_FLIGHT);
	m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = indices.graphicsFamily;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkResult result = vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_commandPools[i]);

		if (result != VK_SUCCESS)
		{
			i3D_logErrorMessage("VULKAN ERROR: Failed to create the command pool for one of the frames in flight. VkResult: %s\n", string_VkResult(result));
			return false;
		}

		VkCommandBufferAllocateInfo commandBufferInfo = {};
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferInfo.commandPool = m_commandPools[i];
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferInfo.commandBufferCount = 1;

		result = vkAllocateCommandBuffers(m_device, &commandBufferInfo, &m_commandBuffers[i]);

		if (result != VK_SUCCESS)
		{
			i3D_logErrorMessage("VULKAN ERROR: Failed to allocate the command buffer for one of the frames in flight. VkResult: %s\n", string_VkResult(result));
			return false;
		}
	}

	VkResult result = vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_immCommandPool);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the command pool used for one-time submits. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	return true;
}

bool i3D_vkRenderingContext::initSwapchainResources()
{	
	if (!m_swapchain.initColorResources(m_device, m_physDevice, m_msaaSamples))
	{
		return false;
	}
	
	if (!m_swapchain.initDepthResources(m_device, m_physDevice, m_immCommandPool, m_graphicsQueue, m_msaaSamples))
	{
		return false;
	}

	return true;
}

bool i3D_vkRenderingContext::initFramebuffers()
{
	m_swapchainFramebuffers.resize(m_swapchain.imageCount);

	for (size_t i = 0; i < m_swapchain.imageCount; i++)
	{
		std::array<VkImageView, 3> attachments =
		{
			m_swapchain.colorImageView,
			m_swapchain.depthImageView,
			m_swapchain.swapchainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_swapchain.imageExtent.width;
		framebufferInfo.height = m_swapchain.imageExtent.height;
		framebufferInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]);

		if (result != VK_SUCCESS)
		{
			i3D_logErrorMessage("VULKAN ERROR: Failed to create one or more of the required framebuffers. VkResult: %s\n", string_VkResult(result));
			return false;
		}
	}

	return true;
}

bool i3D_vkRenderingContext::initModelAndTexture()
{	
	I3D_BASSERT(m_textureClass.initialize_VMA(m_device, m_physDevice, m_allocator, m_immCommandPool, m_graphicsQueue, "assets/images/texture.jpg"));
	I3D_BASSERT(m_meshClass.initialize(m_device, m_physDevice, m_immCommandPool, m_graphicsQueue, "assets/models/export3dcoat.obj"));

	return true;
}

bool i3D_vkRenderingContext::initUniformBuffers()
{
	m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	VkDeviceSize bufferSize = sizeof(vkUniformBufferData);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (!m_uniformBuffers[i].initialize
		(
			m_device,
			m_physDevice,
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		))
		{
			i3D_logErrorMessage("VULKAN ERROR: Failed to create one or more of the uniform buffers.\n");
			return false;
		}

		if (!m_uniformBuffers[i].mapBufferMemory(m_device, 0, bufferSize, 0, &m_uniformBuffersMapped[i]))
		{
			i3D_logErrorMessage("VULKAN ERROR: Failed to map the memory for one or more of the uniform buffers.\n");
			return false;
		}
	}

	return true;
}

bool i3D_vkRenderingContext::initDescriptors()
{
	m_descriptorAllocator.clearPoolSizes();

	// shader.vert: layout (binding = 0) uniform UniformBufferObject{} ubo;
	m_descriptorAllocator.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT);

	// shader.frag: layout (binding = 1) uniform sampler2D texSampler;
	m_descriptorAllocator.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT);

	I3D_BASSERT(m_descriptorAllocator.initPool(m_device, MAX_FRAMES_IN_FLIGHT, 0, nullptr));

	VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] =
	{
		m_descriptorSetLayout, m_descriptorSetLayout
	};

	I3D_BASSERT(m_descriptorAllocator.allocateSets(m_device, MAX_FRAMES_IN_FLIGHT, layouts, nullptr));

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = m_uniformBuffers[i].buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(vkUniformBufferData);

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_textureClass.imageView;
		imageInfo.sampler = m_textureClass.sampler;
		
		std::array<VkWriteDescriptorSet, 2> descriptorWrites = { {} };

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_descriptorAllocator.descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_descriptorAllocator.descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	return true;
}

bool i3D_vkRenderingContext::initSyncObjects()
{
	m_swapchainSemaphores.resize(m_swapchain.imageCount);
	m_frameSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_frameFences.resize(MAX_FRAMES_IN_FLIGHT);
	
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < m_swapchain.imageCount; i++)
	{
		VkResult result = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_swapchainSemaphores[i]);

		if (result != VK_SUCCESS)
		{
			i3D_logErrorMessage("VULKAN ERROR: Failed to create the semaphore for one of the swapchain images. VkResult: %s\n", string_VkResult(result));
			return false;
		}
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkResult result = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_frameSemaphores[i]);

		if (result != VK_SUCCESS)
		{
			i3D_logErrorMessage("VULKAN ERROR: Failed to create the semaphore for one of the frames in flight. VkResult: %s\n", string_VkResult(result));
			return false;
		}

		result = vkCreateFence(m_device, &fenceInfo, nullptr, &m_frameFences[i]);

		if (result != VK_SUCCESS)
		{
			i3D_logErrorMessage("VULKAN ERROR: Failed to create the fence for one of the frames in flight. VkResult: %s\n", string_VkResult(result));
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------------------------------------

void i3D_vkRenderingContext::updateUniformBuffer(uint32_t currentImage)
{
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraRot = glm::vec3(0.0f);

	vkUniformBufferData data = {};
	data.modelMatrix = glm::rotate(glm::mat4(1.0f), m_currentMeshRotation, glm::vec3(0.0f, 1.0f, 0.0f));
	data.viewMatrix = getCameraViewMatrix(cameraPos, cameraRot);
	data.projMatrix = getCameraProjMatrix(glm::radians(60.0f), m_aspectRatio);

	data.cameraPosition = cameraPos;
	data.padding = 0.0f;

	memcpy(m_uniformBuffersMapped[currentImage], &data, sizeof(data));
}

bool i3D_vkRenderingContext::recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkResult result = vkBeginCommandBuffer(buffer, &beginInfo);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to begin the command buffer. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	VkClearValue clearValues[2] = { {} };
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapchain.imageExtent;
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_swapchain.imageExtent.width);
	viewport.height = static_cast<float>(m_swapchain.imageExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(buffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchain.imageExtent;
	vkCmdSetScissor(buffer, 0, 1, &scissor);
	
	if (m_mysteryCounter >= 1000)
	{
		m_meshClass.draw(buffer, m_graphicsPipeline, m_pipelineLayout, m_descriptorAllocator.descriptorSets[m_currentFrame]);
	}
	else
	{
		m_meshClass.draw(buffer, m_graphicsPipeline2, m_pipelineLayout2, m_descriptorAllocator.descriptorSets[m_currentFrame]);
	}

	m_mysteryCounter++;

	if (m_mysteryCounter >= 2000)
	{
		m_mysteryCounter = 0;
	}

	vkCmdEndRenderPass(buffer);

	result = vkEndCommandBuffer(buffer);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to end the command buffer. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	return true;
}

bool i3D_vkRenderingContext::recreateSwapchain()
{
	LONG wndWidth, wndHeight;
	I3D_BASSERT(i3D_windowsUtils::getWindowDimensions(static_cast<HWND>(m_wndMemory), wndWidth, wndHeight));
	
	if ((wndWidth == 0) || (wndHeight == 0))
	{
		// Window is minimized
		return true;
	}

	vkDeviceWaitIdle(m_device);
	cleanupSwapchain();

	I3D_BASSERT(initSwapchain(wndWidth, wndHeight));
	I3D_BASSERT(initSwapchainResources());
	I3D_BASSERT(initFramebuffers());

	return true;
}

// ----------------------------------------------------------------------------------------------------

void i3D_vkRenderingContext::cleanupSwapchain()
{	
	for (auto framebuffer : m_swapchainFramebuffers)
	{
		if (framebuffer != nullptr)
		{
			vkDestroyFramebuffer(m_device, framebuffer, nullptr);
		}
	}

	m_swapchainFramebuffers.clear();
	
	m_swapchain.cleanupDepthResources(m_device);
	m_swapchain.cleanupColorResources(m_device);
	m_swapchain.cleanupSwapchain(m_device);
}

void i3D_vkRenderingContext::cleanupLogicalDevice()
{
	if (m_device != nullptr)
	{
		vkDeviceWaitIdle(m_device);
		
		for (auto& fence : m_frameFences)
		{
			if (fence != nullptr)
			{
				vkDestroyFence(m_device, fence, nullptr);
				fence = nullptr;
			}
		}

		m_frameFences.clear();

		for (auto& semaphore : m_frameSemaphores)
		{
			if (semaphore != nullptr)
			{
				vkDestroySemaphore(m_device, semaphore, nullptr);
				semaphore = nullptr;
			}
		}

		m_frameSemaphores.clear();

		for (auto& semaphore : m_swapchainSemaphores)
		{
			if (semaphore != nullptr)
			{
				vkDestroySemaphore(m_device, semaphore, nullptr);
				semaphore = nullptr;
			}
		}

		m_swapchainSemaphores.clear();

		m_descriptorAllocator.cleanupPool(m_device);
		m_descriptorAllocator.clearSets();

		for (auto& buffer : m_uniformBuffers)
		{
			buffer.unmapBufferMemory(m_device);
			buffer.cleanup(m_device);
		}

		m_uniformBuffers.clear();
		m_uniformBuffersMapped.clear();

		m_meshClass.cleanup(m_device);
		m_textureClass.cleanup(m_device, m_allocator);

		if (m_immCommandPool != nullptr)
		{
			vkDestroyCommandPool(m_device, m_immCommandPool, nullptr);
			m_immCommandPool = nullptr;
		}

		for (auto& pool : m_commandPools)
		{
			if (pool != nullptr)
			{
				vkDestroyCommandPool(m_device, pool, nullptr);
				pool = nullptr;
			}
		}

		m_commandPools.clear();
		m_commandBuffers.clear();

		if (m_graphicsPipeline2 != nullptr)
		{
			vkDestroyPipeline(m_device, m_graphicsPipeline2, nullptr);
			m_graphicsPipeline2 = nullptr;
		}

		if (m_pipelineLayout2 != nullptr)
		{
			vkDestroyPipelineLayout(m_device, m_pipelineLayout2, nullptr);
			m_pipelineLayout2 = nullptr;
		}

		if (m_graphicsPipeline != nullptr)
		{
			vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
			m_graphicsPipeline = nullptr;
		}
		
		if (m_pipelineLayout != nullptr)
		{
			vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
			m_pipelineLayout = nullptr;
		}

		if (m_descriptorSetLayout != nullptr)
		{
			vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
			m_descriptorSetLayout = nullptr;
		}

		if (m_renderPass != nullptr)
		{
			vkDestroyRenderPass(m_device, m_renderPass, nullptr);
			m_renderPass = nullptr;
		}

		cleanupSwapchain();

		if (m_allocator != nullptr)
		{
			vmaDestroyAllocator(m_allocator);
			m_allocator = nullptr;
		}

		vkDestroyDevice(m_device, nullptr);
		m_device = nullptr;
	}
}

void i3D_vkRenderingContext::cleanupInstance()
{
	if (m_instance != nullptr)
	{
		if (m_surface != nullptr)
		{
			vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
			m_surface = nullptr;
		}

#ifdef I3D_VULKAN_VALIDATION
		if (m_debugMessenger != nullptr)
		{
			i3D_vkValidation::destroyDebugMessenger(m_instance, m_debugMessenger, nullptr);
			m_debugMessenger = nullptr;
		}
#endif

		vkDestroyInstance(m_instance, nullptr);
		m_instance = nullptr;
	}

	m_wndMemory = nullptr;
}