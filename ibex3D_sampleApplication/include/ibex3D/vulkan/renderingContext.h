#pragma once

#include "swapchainObject.h"
#include "descriptors.h"
#include "meshObject.h"
#include "textureObject.h"
#include "bufferObject.h"

#include <thirdparty/vma/vk_mem_alloc.h>

// ----------------------------------------------------------------------------------------------------

class i3D_vkRenderingContext
{
public:
	bool initialize(void* wndMemory);
	bool drawFrame(float meshRotation);
	void refresh();
	void cleanup();

private:
	bool initInstance();
	bool initSurface(void* wndMemory);
	bool initPhysicalDevice();
	bool initLogicalDevice();
	bool initVMA();
	bool initSwapchain(int wndWidth, int wndHeight);
	bool initRenderPass();
	bool initDescriptorSetLayout();
	bool initGraphicsPipeline();
	bool initCommands();
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

private:
	void* m_wndMemory = nullptr;
	VkInstance m_instance = nullptr;
	VkDebugUtilsMessengerEXT m_debugMessenger = nullptr;
	VkSurfaceKHR m_surface = nullptr;
	VkPhysicalDevice m_physDevice = nullptr;
	VkDevice m_device = nullptr;
	VmaAllocator m_allocator = nullptr;
	VkQueue m_graphicsQueue = nullptr;
	VkQueue m_presentQueue = nullptr;
	i3D_vkSwapchainObject m_swapchain;
	VkRenderPass m_renderPass = nullptr;
	i3D_vkDescriptorAllocator m_descriptorAllocator;
	VkDescriptorSetLayout m_descriptorSetLayout = nullptr;
	VkPipelineLayout m_pipelineLayout = nullptr;
	VkPipeline m_graphicsPipeline = nullptr;
	VkCommandPool m_immCommandPool = nullptr;

	VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	uint32_t m_currentFrame = 0;
	
	std::vector<VkFramebuffer> m_swapchainFramebuffers;
	std::vector<i3D_vkBufferObject> m_uniformBuffers;
	std::vector<void*> m_uniformBuffersMapped;
	std::vector<VkCommandPool> m_commandPools;
	std::vector<VkCommandBuffer> m_commandBuffers;
	std::vector<VkSemaphore> m_swapchainSemaphores;
	std::vector<VkSemaphore> m_frameSemaphores;
	std::vector<VkFence> m_frameFences;

	i3D_vkTextureObject m_textureClass;
	i3D_vkMeshObject m_meshClass;
	i3D_vkMeshObject m_meshClass2;

	float m_aspectRatio = 1.0f;
	float m_currentMeshRotation = 0.0f;
	bool m_refreshSwapchain = false;
};