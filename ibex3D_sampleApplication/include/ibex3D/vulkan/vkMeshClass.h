#pragma once

#include <vulkan/vulkan.h>

#include <ibex3D/math/vec2.h>
#include <ibex3D/math/vec3.h>

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