#pragma once
#include <vulkan/vulkan.h>
#include <vector>

struct vkVertexBufferClass;

class vk_renderingContext
{
public:
	bool initialize(const char* appName, void* wndMemory);
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
	bool initGraphicsPipeline();
	bool initFramebuffers();
	bool initCommandPool();
	bool initVertexBuffer();
	bool initCommandBuffers();
	bool initSyncObjects();

	bool recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex);
	void recreateSwapchain();

	void cleanupSwapchain(VkDevice logicalDevice);
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
	VkFormat m_swapchainImageFormat;
	VkExtent2D m_swapchainExtent = {};
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	std::vector<VkFramebuffer> m_swapchainFramebuffers;

	VkRenderPass m_renderPass = nullptr;
	VkPipelineLayout m_pipelineLayout = nullptr;
	VkPipeline m_graphicsPipeline = nullptr;
	VkCommandPool m_commandPool = nullptr;
	uint32_t m_currentFrame = 0;
	std::vector<VkCommandBuffer> m_commandBuffers;
	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;

	vkVertexBufferClass* m_vertexBuffer = nullptr;
};