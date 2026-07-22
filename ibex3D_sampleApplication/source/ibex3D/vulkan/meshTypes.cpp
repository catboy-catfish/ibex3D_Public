#include <ibex3D/vulkan/meshTypes.h>



VkVertexInputBindingDescription i3D_vkVertex::getBindingDesc()
{
	VkVertexInputBindingDescription bindingDesc = {};

	bindingDesc.binding = 0;
	bindingDesc.stride = sizeof(i3D_vkVertex);
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDesc;
}

std::array<VkVertexInputAttributeDescription, 3> i3D_vkVertex::getAttributeDescs()
{
	std::array<VkVertexInputAttributeDescription, 3> attribDescs = {};

	// Position
	attribDescs[0].binding = 0;
	attribDescs[0].location = 0;
	attribDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDescs[0].offset = offsetof(i3D_vkVertex, vertexPosition);

	// Normals
	attribDescs[1].binding = 0;
	attribDescs[1].location = 1;
	attribDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDescs[1].offset = offsetof(i3D_vkVertex, vertexNormal);

	// Texture cordinates
	attribDescs[2].binding = 0;
	attribDescs[2].location = 2;
	attribDescs[2].format = VK_FORMAT_R32G32_SFLOAT;
	attribDescs[2].offset = offsetof(i3D_vkVertex, textureCoord);

	return attribDescs;
}