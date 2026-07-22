#pragma once

#include "bufferObject.h"
#include "meshTypes.h"

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