#pragma once

#include <vulkan/vulkan.h>

#include <ibex3D/math/vec2.h>
#include <ibex3D/math/vec3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <array>
#include <vector>

struct vkVertex
{
	vec3 position;
	vec3 color;
	vec2 texCoord;

	// ----------------------------------------------------------------------------------------------------
	
	bool operator == (const vkVertex& other) const
	{
		return (position == other.position) && (color == other.color) && (texCoord == other.texCoord);
	}
	
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

struct vkTextureClass;
struct vkMeshClass
{
	// ----------------------------------------------------------------------------------------------------
	
	VkBuffer vtxBuffer = nullptr;
	VkBuffer idxBuffer = nullptr;
	VkDeviceMemory vtxBufferMemory = nullptr;
	VkDeviceMemory idxBufferMemory = nullptr;

	std::vector<vkVertex> vertices;
	std::vector<uint32_t> indices;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;
	
	float currentRotation = 0.0f;

	// ----------------------------------------------------------------------------------------------------

	void initSimpleModel();
	bool loadObjFromFile(const char* objFilePath);

	bool initVertexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue);
	bool initIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue);
	bool initUniformBuffers(VkDevice device, VkPhysicalDevice physDevice);

	void updateUniformBuffer(uint32_t currentImg, const VkExtent2D& scExtent);

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* meshFilePath, vkTextureClass* texture);
	void setMeshRotation(float rotation);
	void draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet);
	void cleanup(VkDevice device);
};