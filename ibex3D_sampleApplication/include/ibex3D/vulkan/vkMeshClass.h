#pragma once

#include <array>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

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

struct vkMeshClass
{	
	VkBuffer vtxBuffer = nullptr;
	VkBuffer idxBuffer = nullptr;
	VkDeviceMemory vtxBufferMemory = nullptr;
	VkDeviceMemory idxBufferMemory = nullptr;

	std::vector<vkVertex> vertices;
	std::vector<uint32_t> indices;

	// ----------------------------------------------------------------------------------------------------

	void initSimpleModel();
	bool loadObjFromFile(const char* objFilePath);

	bool initVertexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue);
	bool initIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue);

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* meshFilePath);
	void draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet);
	void cleanup(VkDevice device);
};