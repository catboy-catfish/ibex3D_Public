#pragma once

#include <ibex3D/vulkan/bufferObject.h>

#include <array>
#include <vector>

#include <glm/glm.hpp>

struct vkVertex
{
	glm::vec3 vertexPosition;
	glm::vec3 vertexColor;
	glm::vec3 vertexNormal;
	glm::vec2 textureCoord;

	// ----------------------------------------------------------------------------------------------------
	
	bool operator == (const vkVertex& other) const
	{
		return	(vertexPosition == other.vertexPosition) && 
				(vertexColor == other.vertexColor) && 
				(vertexNormal == other.vertexNormal) &&
				(textureCoord == other.textureCoord);
	}
	
	// ----------------------------------------------------------------------------------------------------

	static VkVertexInputBindingDescription getBindingDesc();
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescs();
};

struct vkMeshObject
{	
	vkBufferObject vtxIdxBuffer;
	VkDeviceSize vtxBufferSize = 0;

	std::vector<vkVertex> vertices;
	std::vector<uint32_t> indices;

	// ----------------------------------------------------------------------------------------------------

	void initSimpleModel();
	bool loadObjFromFile(const char* objFilePath);

	bool initVertexIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue);

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* meshFilePath);
	void draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet);
	void cleanup(VkDevice device);
};