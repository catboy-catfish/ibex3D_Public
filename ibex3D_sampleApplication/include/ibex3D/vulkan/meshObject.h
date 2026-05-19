#pragma once

#include "bufferObject.h"

#include <thirdparty/glm/glm.hpp>

#include <array>
#include <vector>

// - Helper struct declaration ------------------------------------------------------------------------

struct vkVertex
{
	glm::vec3 vertexPosition;
	glm::vec3 vertexNormal;
	glm::vec2 textureCoord;

	// - Operator overrides -------------------------------------------------------------------------------
	
	bool operator == (const vkVertex& other) const
	{
		return (vertexPosition == other.vertexPosition)
			&& (vertexNormal == other.vertexNormal)
			&& (textureCoord == other.textureCoord);
	}
	
	// - Functions ----------------------------------------------------------------------------------------

	static VkVertexInputBindingDescription getBindingDesc();
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescs();
};

// - Main struct declaration --------------------------------------------------------------------------

struct vkMeshObject
{	
	vkBufferObject vtxIdxBuffer;
	VkDeviceSize vtxBufferSize = 0;

	std::vector<vkVertex> vertices;
	std::vector<uint32_t> indices;

	// - Functions ----------------------------------------------------------------------------------------

	void initSimpleModel();
	bool loadObjFromFile(const char* objFilePath);

	bool initVertexIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue);

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* meshFilePath);
	void draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet);
	void cleanup(VkDevice device);
};