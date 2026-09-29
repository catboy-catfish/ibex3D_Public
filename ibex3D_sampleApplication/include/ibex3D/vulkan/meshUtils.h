#pragma once

#include <ibex3D/meshes/meshLoading.h>

#include <array>

#include <vulkan/vulkan.h>

// ----------------------------------------------------------------------------------------------------

static VkVertexInputBindingDescription i3D_vkGetVertexBindingDesc()
{
	VkVertexInputBindingDescription bindingDesc = {};

	bindingDesc.binding = 0;
	bindingDesc.stride = sizeof(i3D_vertex);
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDesc;
}

static std::array<VkVertexInputAttributeDescription, 3> i3D_vkGetVertexAttributeDescs()
{
	std::array<VkVertexInputAttributeDescription, 3> attribDescs = {};

	// Position
	attribDescs[0].binding = 0;
	attribDescs[0].location = 0;
	attribDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDescs[0].offset = static_cast<uint32_t>(i3D_vertex::getPositionByteOffset());

	// Normals
	attribDescs[1].binding = 0;
	attribDescs[1].location = 1;
	attribDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDescs[1].offset = static_cast<uint32_t>(i3D_vertex::getNormalByteOffset());

	// Texture cordinates
	attribDescs[2].binding = 0;
	attribDescs[2].location = 2;
	attribDescs[2].format = VK_FORMAT_R32G32_SFLOAT;
	attribDescs[2].offset = static_cast<uint32_t>(i3D_vertex::getTexCoordByteOffset());

	return attribDescs;
}