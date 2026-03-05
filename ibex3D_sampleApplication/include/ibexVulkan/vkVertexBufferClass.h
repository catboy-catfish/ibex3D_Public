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
	static std::array<VkVertexInputAttributeDescription, 2> getAttribDescs();
};

struct vkVertexBufferClass
{
	std::vector<vkVertex> vertices;
	VkBuffer vtxBuffer = nullptr;
	VkDeviceMemory vtxBufferMemory = nullptr;

	void initVertices();

	bool initialize(VkPhysicalDevice physicalDevice, VkDevice logicalDevice);
	void cleanup(VkDevice logicalDevice);
};