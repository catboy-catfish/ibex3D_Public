#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>
#include <vector>

struct vkVertex
{
	glm::vec2 position;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDesc();
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescs();
};

struct vkMeshClass
{
	std::vector<vkVertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vtxBuffer = nullptr;
	VkBuffer idxBuffer = nullptr;
	VkDeviceMemory vtxBufferMemory = nullptr;
	VkDeviceMemory idxBufferMemory = nullptr;

	void initMeshData();
	bool initVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	bool initIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

	bool initialize(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
	void cleanup(VkDevice logicalDevice);
};