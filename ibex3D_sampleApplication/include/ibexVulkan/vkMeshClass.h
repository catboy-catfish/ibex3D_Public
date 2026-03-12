#pragma once

#define GLM_FORCE_RADIANS
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>
#include <vector>

struct vkVertex
{
	glm::vec2 position;
	glm::vec2 texCoord;
	glm::vec3 color;

	// ----------------------------------------------------------------------------------------------------

	static VkVertexInputBindingDescription getBindingDesc();
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescs();
};

struct vkUniformBufferData
{
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
};

struct vkMeshClass
{
	// ----------------------------------------------------------------------------------------------------
	
	std::vector<vkVertex> vertices;
	VkBuffer vtxBuffer = nullptr;
	VkDeviceMemory vtxBufferMemory = nullptr;

	std::vector<uint32_t> indices;
	VkBuffer idxBuffer = nullptr;
	VkDeviceMemory idxBufferMemory = nullptr;

	std::vector<VkBuffer> uniBuffers;
	std::vector<VkDeviceMemory> uniBuffersMemory;
	std::vector<void*> uniBuffersMapped;

	VkDescriptorSetLayout descriptorSetLayout = nullptr;
	VkDescriptorPool descriptorPool = nullptr;
	std::vector<VkDescriptorSet> descriptorSets;

	// ----------------------------------------------------------------------------------------------------
	
	float currentRotation = 0.0f;

	// ----------------------------------------------------------------------------------------------------

	void initMeshData();
	bool initVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	bool initIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	bool initUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, size_t maxFramesInFlight);
	bool initDescriptorSetLayout(VkDevice logicalDevice);
	bool initDescriptorPoolAndSets(VkDevice logicalDevice, VkImageView textureImageView, VkSampler textureSampler, size_t maxFramesInFlight);

	void updateUniformBuffer(uint32_t currentImage, const VkExtent2D& swapchainExtent);

	bool initialize(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkImageView textureImageView, VkSampler textureSampler, VkCommandPool commandPool, VkQueue graphicsQueue, size_t maxFramesInFlight);
	void setMeshRotation(float rotation);
	void draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);
	void cleanup(VkDevice logicalDevice);
};