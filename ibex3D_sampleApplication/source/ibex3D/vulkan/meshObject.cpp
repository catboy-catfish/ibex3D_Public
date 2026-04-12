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

	auto numVertices = vertices.size();
	auto numIndices = indices.size();

	return true;
}

bool vkMeshObject::initVertexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue)
{
	VkDeviceSize bufferSize = vertices.size() * sizeof(vertices[0]);

	vkBufferObject stagingBuffer;

	if (!stagingBuffer.initialize
	(
		device,
		physDevice,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	))
	{
		vkUtils::printVkError("vkMeshObject::initVertexBuffer()", "Couldn't create the staging buffer.");
		return false;
	}

	if (!stagingBuffer.updateBufferData(device, bufferSize, vertices.data()))
	{
		vkUtils::printVkError("vkMeshObject::initVertexBuffer()", "Couldn't set the staging buffer data.");
		stagingBuffer.cleanup(device);
		return false;
	}

	if (!vertexBuffer.initialize
	(
		device,
		physDevice,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	))
	{
		vkUtils::printVkError("vkMeshObject::initVertexBuffer()", "Couldn't create the vertex buffer.");
		stagingBuffer.cleanup(device);
		return false;
	}

	if (!vertexBuffer.cmdCopyBuffer(device, cmdPool, gfxQueue, bufferSize, stagingBuffer.buffer))
	{
		vkUtils::printVkError("vkMeshObject::initVertexBuffer()", "Couldn't copy the staging buffer memory to the vertex buffer.");
		stagingBuffer.cleanup(device);
		return false;
	}

	stagingBuffer.cleanup(device);
	return true;
}

bool vkMeshObject::initIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue)
{
	VkDeviceSize bufferSize = indices.size() * sizeof(indices[0]);

	vkBufferObject stagingBuffer;

	if (!stagingBuffer.initialize
	(
		device,
		physDevice,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	))
	{
		vkUtils::printVkError("vkMeshObject::initIndexBuffer()", "Couldn't create the staging buffer.");
		return false;
	}

	if (!stagingBuffer.updateBufferData(device, bufferSize, indices.data()))
	{
		vkUtils::printVkError("vkMeshObject::initIndexBuffer()", "Couldn't set the staging buffer data.");
		stagingBuffer.cleanup(device);
		return false;
	}

	if (!indexBuffer.initialize
	(
		device,
		physDevice,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	))
	{
		vkUtils::printVkError("vkMeshObject::initIndexBuffer()", "Couldn't create the index buffer.");
		stagingBuffer.cleanup(device);
		return false;
	}

	if (!indexBuffer.cmdCopyBuffer(device, cmdPool, gfxQueue, bufferSize, stagingBuffer.buffer))
	{
		vkUtils::printVkError("vkMeshObject::initIndexBuffer()", "Couldn't copy the staging buffer memory to the index buffer.");
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
	
	if (!initVertexBuffer(device, physDevice, cmdPool, gfxQueue))
	{
		return false;
	}

	if (!initIndexBuffer(device, physDevice, cmdPool, gfxQueue))
	{
		return false;
	}

	return true;
}

void vkMeshObject::draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet)
{
	VkBuffer vertexBuffers[] = { vertexBuffer.buffer };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(buffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	vkCmdDrawIndexed(buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void vkMeshObject::cleanup(VkDevice device)
{	
	indexBuffer.cleanup(device);
	vertexBuffer.cleanup(device);
}