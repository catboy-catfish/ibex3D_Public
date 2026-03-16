#pragma once

#include <ibexVulkan/vkTextureClass.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <array>
#include <vector>

struct vkVertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;

	// ----------------------------------------------------------------------------------------------------
	
	bool operator == (const vkVertex& other) const;
	
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
	
	vkTextureClass textureClass;

	std::vector<vkVertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vtxBuffer = nullptr;
	VkBuffer idxBuffer = nullptr;
	std::vector<VkBuffer> uniBuffers;

	VkDeviceMemory vtxBufferMemory = nullptr;
	VkDeviceMemory idxBufferMemory = nullptr;
	std::vector<VkDeviceMemory> uniBuffersMemory;
	std::vector<void*> uniBuffersMapped;

	VkDescriptorSetLayout descriptorSetLayout = nullptr;
	VkDescriptorPool descriptorPool = nullptr;
	std::vector<VkDescriptorSet> descriptorSets;

	// ----------------------------------------------------------------------------------------------------
	
	float currentRotation = 0.0f;

	// ----------------------------------------------------------------------------------------------------

	bool initTexture(VkDevice device, VkPhysicalDevice physDevice, const char* textureFilePath, VkCommandPool cmdPool, VkQueue gfxQueue);
	bool initModel(const char* meshFilePath);
	bool initVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	bool initIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	bool initUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, size_t maxFramesInFlight);
	bool initDescriptorSetLayout(VkDevice logicalDevice);
	bool initDescriptorPoolAndSets(VkDevice logicalDevice, VkImageView textureImageView, VkSampler textureSampler, size_t maxFramesInFlight);

	void updateUniformBuffer(uint32_t currentImage, const VkExtent2D& swapchainExtent);

	bool initialize(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, size_t maxFramesInFlight);
	void setMeshRotation(float rotation);
	void draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);
	void cleanup(VkDevice logicalDevice);
};