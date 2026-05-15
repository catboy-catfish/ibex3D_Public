#pragma once
#include "swapchainObject.h"
#include "meshObject.h"
#include "textureObject.h"
#include "bufferObject.h"

class vkRenderingContext
{
public:
	bool initialize(void* wndMemory);
	void setMeshRotation(float rotation);
	bool drawFrame();
	void enableResizedFlag();
	void cleanup();

private:
	bool initInstance();
	bool initSurface(void* wndMemory);
	bool initPhysicalDevice(VkSampleCountFlagBits msaaSamplesUsed);
	bool initLogicalDevice();
	bool initSwapchain(int wndWidth, int wndHeight);
	bool initRenderPass();
	bool initDescriptorSetLayout();
	bool initGraphicsPipeline();
	bool initCommandPoolAndBuffers();
	bool initSwapchainResources();
	bool initFramebuffers();
	bool initModelAndTexture();
	bool initUniformBuffers();
	bool initDescriptorPoolAndSets();
	bool initSyncObjects();

	void updateUniformBuffer(uint32_t currentImage);
	bool recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex);
	bool recreateSwapchain();

	void cleanupSwapchain();
	void cleanupLogicalDevice();
	void cleanupInstance();
	
	static bool checkInstanceLayerSupport();
	static bool checkPhysDeviceExtensionSupport(VkPhysicalDevice physDevice);
	static void printAvailableInstanceExtensions();
	static void printAvailableInstanceLayers();

	static std::vector<const char*> getRequiredInstanceExtensions();
	static std::vector<const char*> getRequiredInstanceLayers();
	static std::vector<const char*> getRequiredDeviceExtensions();

private:
	void* m_wndMemory = nullptr;
	VkInstance m_instance = nullptr;
	VkDebugUtilsMessengerEXT m_debugMessenger = nullptr;
	VkSurfaceKHR m_surface = nullptr;
	VkPhysicalDevice m_physicalDevice = nullptr;
	VkDevice m_logicalDevice = nullptr;
	VkQueue m_graphicsQueue = nullptr;
	VkQueue m_presentQueue = nullptr;
	vkSwapchainObject m_swapchain;
	VkRenderPass m_renderPass = nullptr;
	VkDescriptorSetLayout m_descriptorSetLayout = nullptr;
	VkPipelineLayout m_pipelineLayout = nullptr;
	VkPipeline m_graphicsPipeline = nullptr;
	VkCommandPool m_commandPool = nullptr;
	VkDescriptorPool m_descriptorPool = nullptr;

	VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	uint32_t m_currentFrame = 0;
	
	std::vector<VkFramebuffer> m_swapchainFramebuffers;
	std::vector<vkBufferObject> m_uniformBuffers;
	std::vector<void*> m_uniformBuffersMapped;
	std::vector<VkDescriptorSet> m_descriptorSets;
	std::vector<VkCommandBuffer> m_commandBuffers;
	std::vector<VkSemaphore> m_swapchainSemaphores;
	std::vector<VkSemaphore> m_frameSemaphores;
	std::vector<VkFence> m_frameFences;

	vkTextureObject m_textureClass;
	vkMeshObject m_meshClass;

	float m_aspectRatio = 1.0f;
	float m_currentMeshRotation = 0.0f;
	bool m_wasJustResized = false;
	bool m_useVsync = true;
};