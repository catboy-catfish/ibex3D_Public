#pragma once

#include "bufferObject.h"

#include <thirdparty/glm/vec2.hpp>
#include <thirdparty/glm/vec3.hpp>

// ----------------------------------------------------------------------------------------------------

struct i3D_vkMeshObject
{	
	i3D_vkBufferObject vtxIdxBuffer;
	VkDeviceSize vtxBufferSize = 0;
	uint32_t indexCount = 0;
	
	// ----------------------------------------------------------------------------------------------------

	bool initVertexIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* meshFilePath);

	bool initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* meshFilePath);
	void draw(VkCommandBuffer buffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, VkDescriptorSet* pDescriptorSet);
	void cleanup(VkDevice device);
};