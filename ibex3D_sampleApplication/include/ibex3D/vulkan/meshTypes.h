#pragma once

#include <vulkan/vulkan.h>

#include <array>

#include <thirdparty/glm/glm.hpp>

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