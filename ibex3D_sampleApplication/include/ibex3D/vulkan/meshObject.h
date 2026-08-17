#pragma once

#include "bufferObject.h"

#include <array>
#include <vector>

#include <thirdparty/glm/vec2.hpp>
#include <thirdparty/glm/vec3.hpp>

// ----------------------------------------------------------------------------------------------------

struct i3D_vkVertex
{
	glm::vec3 vertexPosition;
	glm::vec3 vertexNormal;
	glm::vec2 textureCoord;

	// ----------------------------------------------------------------------------------------------------

	bool operator == (const i3D_vkVertex& other) const
	{
		return (vertexPosition == other.vertexPosition)
			&& (vertexNormal == other.vertexNormal)
			&& (textureCoord == other.textureCoord);
	}

	// ----------------------------------------------------------------------------------------------------

	static VkVertexInputBindingDescription getBindingDesc();
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescs();
};

// ----------------------------------------------------------------------------------------------------

struct i3D_vkMeshObject
{	
	i3D_vkBufferObject vtxIdxBuffer;
	VkDeviceSize vtxBufferSize = 0;

	std::vector<i3D_vkVertex> vertices;
	std::vector<uint32_t> indices;

	// ----------------------------------------------------------------------------------------------------

	void initSimpleModel();
	bool loadObjFromFile(const char* objFilePath);

	bool initVertexIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue);

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* meshFilePath);
	void draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet);
	void cleanup(VkDevice device);
};