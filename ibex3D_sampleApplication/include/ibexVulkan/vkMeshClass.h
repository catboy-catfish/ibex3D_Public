#pragma once

#include <vulkan/vulkan.h>

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
	
	bool operator == (const vkVertex& other) const
	{
		return (position == other.position) && (color == other.color) && (texCoord == other.texCoord);
	}
	
	// ----------------------------------------------------------------------------------------------------

	static VkVertexInputBindingDescription getBindingDesc();
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescs();
};

struct vkParticle
{
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec4 color;
};

struct vkUniformBufferData
{
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
};

struct vkTextureClass;
struct vkMeshClass
{
	// ----------------------------------------------------------------------------------------------------
	
	VkBuffer vtxBuffer = nullptr;
	VkBuffer idxBuffer = nullptr;
	VkDeviceMemory vtxBufferMemory = nullptr;
	VkDeviceMemory idxBufferMemory = nullptr;
	VkDescriptorSetLayout computeDescriptorSetLayout = nullptr;
	VkDescriptorSetLayout descriptorSetLayout = nullptr;
	VkDescriptorPool descriptorPool = nullptr;

	std::vector<vkVertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;
	std::vector<VkBuffer> shaderStorageBuffers;
	std::vector<VkDeviceMemory> shaderStorageBuffersMemory;
	std::vector<VkDescriptorSet> computeDescriptorSets;
	std::vector<VkDescriptorSet> descriptorSets;
	
	float currentRotation = 0.0f;
	
	// ----------------------------------------------------------------------------------------------------

	bool initModel(const char* meshFilePath);
	bool initVertexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue);
	bool initIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue);
	bool initUniformBuffers(VkDevice device, VkPhysicalDevice physDevice, size_t maxFramesInFlight);
	bool initShaderStorageBuffers(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, size_t maxFramesInFlight, int windowWidth, int windowHeight);
	bool initComputeDescriptorSetLayout(VkDevice device);
	bool initDescriptorSetLayout(VkDevice device);
	bool initDescriptorPoolAndSets(VkDevice device, vkTextureClass* texture, size_t maxFramesInFlight);

	void updateUniformBuffer(uint32_t currentImg, const VkExtent2D& scExtent);

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, size_t maxFramesInFlight, int windowWidth, int windowHeight, const char* meshFilePath, vkTextureClass* texture);
	void setMeshRotation(float rotation);
	void draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);
	void dispatch(VkCommandBuffer cmdBuffer, VkCommandBuffer computeCmdBuffer, VkPipeline computePipeline, VkPipelineLayout pipelineLayout, int frame);
	void cleanup(VkDevice device);
};