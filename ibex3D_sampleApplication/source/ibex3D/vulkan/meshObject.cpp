#include <ibex3D/vulkan/meshObject.h>
#include <ibex3D/vulkan/utils.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include <string>
#include <unordered_map>

namespace std
{
	template<> struct hash<vkVertex>
	{
		size_t operator()(vkVertex const& vertex) const
		{
			// The ^ is the bitwise XOR operator
			// TODO: Look into hashing to better understand this bullshit

			return	((hash<glm::vec3>()(vertex.position) ^ 
					(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ 
					(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

VkVertexInputBindingDescription vkVertex::getBindingDesc()
{
	VkVertexInputBindingDescription bindingDesc = {};

	bindingDesc.binding = 0;
	bindingDesc.stride = sizeof(vkVertex);
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDesc;
}

std::array<VkVertexInputAttributeDescription, 3> vkVertex::getAttributeDescs()
{
	std::array<VkVertexInputAttributeDescription, 3> attribDescs = {};
	
	// Position
	attribDescs[0].binding = 0;
	attribDescs[0].location = 0;
	attribDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDescs[0].offset = offsetof(vkVertex, position);

	// Color
	attribDescs[1].binding = 0;
	attribDescs[1].location = 1;
	attribDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDescs[1].offset = offsetof(vkVertex, color);

	// Texture cordinates
	attribDescs[2].binding = 0;
	attribDescs[2].location = 2;
	attribDescs[2].format = VK_FORMAT_R32G32_SFLOAT;
	attribDescs[2].offset = offsetof(vkVertex, texCoord);

	return attribDescs;
}

// ----------------------------------------------------------------------------------------------------

void vkMeshObject::initSimpleModel()
{
	vertices =
	{
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	};

	indices =
	{
		0, 1, 2, 2, 3, 0
	};
}

bool vkMeshObject::loadObjFromFile(const char* objFilePath)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objFilePath))
	{
		vkUtils::printVkError("vkMeshObject::loadObjFromFile()", "Couldn't load the model file.");
		return false;
	}

	std::unordered_map<vkVertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			vkVertex vertex = {};

			int startVertexIdx = 3 * index.vertex_index;
			int startTexCoordIdx = 2 * index.texcoord_index;

			vertex.position =
			{
				attrib.vertices[startVertexIdx],
				attrib.vertices[startVertexIdx + 1],
				attrib.vertices[startVertexIdx + 2]
			};

			vertex.texCoord =
			{
				attrib.texcoords[startTexCoordIdx],
				1.0f - attrib.texcoords[startTexCoordIdx + 1],
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

	return true;
}

bool vkMeshObject::initVertexIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue)
{
	vtxBufferSize = sizeof(vertices[0]) * vertices.size();
	
	VkDeviceSize idxBufferSize = sizeof(indices[0]) * indices.size();
	VkDeviceSize combinedBufferSize = vtxBufferSize + idxBufferSize;

	vkBufferObject stagingBuffer;

	if (!stagingBuffer.initialize
	(
		device,
		physDevice,
		combinedBufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	))
	{
		vkUtils::printVkError("vkMeshObject::initVertexIndexBuffer()", "Couldn't create the staging buffer.");
		return false;
	}

	void* data;
	vkMapMemory(device, stagingBuffer.bufferMemory, 0, combinedBufferSize, 0, &data);

	memcpy(data, vertices.data(), vtxBufferSize);
	memcpy(static_cast<char*>(data) + vtxBufferSize, indices.data(), idxBufferSize);	// "static_cast<char*>(data) + vtxBufferSize" is pointer arithmetic, right?

	vkUnmapMemory(device, stagingBuffer.bufferMemory);

	if (!vtxIdxBuffer.initialize
	(
		device,
		physDevice,
		combinedBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	))
	{
		vkUtils::printVkError("vkMeshObject::initVertexIndexBuffer()", "Couldn't create the combined vertex-index buffer.");
		stagingBuffer.cleanup(device);
		return false;
	}

	if (!vtxIdxBuffer.cmdCopyBuffer(device, cmdPool, gfxQueue, combinedBufferSize, stagingBuffer.buffer))
	{
		vkUtils::printVkError("vkMeshObject::initVertexIndexBuffer()", "Couldn't copy the staging memory to the combined vertex-index buffer.");
		stagingBuffer.cleanup(device);
		return false;
	}

	stagingBuffer.cleanup(device);
	return true;
}

bool vkMeshObject::initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* meshFilePath)
{
	if (!loadObjFromFile(meshFilePath))
	{
		return false;
	}
	
	if (!initVertexIndexBuffer(device, physDevice, cmdPool, gfxQueue))
	{
		return false;
	}

	return true;
}

void vkMeshObject::draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet)
{
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(buffer, 0, 1, &vtxIdxBuffer.buffer, offsets);
	vkCmdBindIndexBuffer(buffer, vtxIdxBuffer.buffer, vtxBufferSize, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	vkCmdDrawIndexed(buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void vkMeshObject::cleanup(VkDevice device)
{	
	vtxIdxBuffer.cleanup(device);
}