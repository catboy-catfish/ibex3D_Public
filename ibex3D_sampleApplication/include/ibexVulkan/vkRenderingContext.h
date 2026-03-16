#pragma once
#include <ibexVulkan/vkMeshClass.h>

class vkRenderingContext
{
public:
	bool initialize(const char* appName, void* wndMemory);
	void setMeshRotation(float rotation);
	bool drawFrame();
	void enableResizedFlag();
	void cleanup();

private:
	bool initInstance(const char* appName);
	bool initSurface(void* wndMemory);
	bool initPhysicalDevice();
	bool initLogicalDevice();
	bool initSwapchain(int wndWidth, int wndHeight);
	bool initRenderPass();
	bool initDescriptorSetLayout();
	bool initGraphicsPipeline();
	bool initCommandPool();
	bool initDepthResources();
	bool initFramebuffers();
	bool initMeshClass();
	bool initCommandBuffers();
	bool initSyncObjects();

	bool recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex);
	void recreateSwapchain();

	void cleanupSwapchain(VkDevice device);
	void cleanupLogicalDevice();
	void cleanupInstance();

	bool checkInstanceLayerSupport();
	bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice device);
	void printAvailableInstanceExtensions();
	void printAvailableInstanceLayers();

private:
	void* m_wndMemory = nullptr;

	bool m_wasJustResized = false;

	VkInstance m_instance = nullptr;
	VkDebugUtilsMessengerEXT m_debugMessenger = nullptr;
	VkSurfaceKHR m_surface = nullptr;
	VkPhysicalDevice m_physicalDevice = nullptr;
	VkDevice m_logicalDevice = nullptr;
	VkQueue m_graphicsQueue = nullptr;
	VkQueue m_presentQueue = nullptr;

	VkSwapchainKHR m_swapchain = nullptr;
	VkFormat m_swapchainImageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D m_swapchainExtent = {};
	uint32_t m_swapchainImageCount = 0;
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	std::vector<VkFramebuffer> m_swapchainFramebuffers;
	VkRenderPass m_renderPass = nullptr;
	VkPipelineLayout m_pipelineLayout = nullptr;
	VkPipeline m_graphicsPipeline = nullptr;

	uint32_t m_currentFrame = 0;
	VkCommandPool m_commandPool = nullptr;
	std::vector<VkCommandBuffer> m_commandBuffers;
	std::vector<VkSemaphore> m_swapchainSemaphores;
	std::vector<VkSemaphore> m_frameSemaphores;
	std::vector<VkFence> m_frameFences;

	vkMeshClass m_meshClass;

	VkImage m_depthImage = nullptr;
	VkDeviceMemory m_depthImageMemory = nullptr;
	VkImageView m_depthImageView = nullptr;
};