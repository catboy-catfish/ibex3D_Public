#include <ibex3D/vulkan/meshObject.h>

#include <stdio.h>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <thirdparty/tinyobjloader/tiny_obj_loader.h>	// includes map, string and vector

#define GLM_ENABLE_EXPERIMENTAL
#include <thirdparty/glm/gtx/hash.hpp>

// ----------------------------------------------------------------------------------------------------

namespace std
{
	template<> struct hash<i3D_vkVertex>
	{
		size_t operator()(i3D_vkVertex const& vertex) const
		{
			// The ^ is the bitwise XOR operator.
			// TODO: Look into hashing to better understand this bullshit. https://en.cppreference.com/cpp/utility/hash

			return ((hash<glm::vec3>()(vertex.vertexPosition) ^ (hash<glm::vec3>()(vertex.vertexNormal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.textureCoord) << 1);
		}
	};
}

// ----------------------------------------------------------------------------------------------------

void i3D_vkMeshObject::initSimpleModel()
{
	vertices =
	{
		{ { -0.5f, -0.5f,  0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { -0.5f,  0.5f,  0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } }
	};

	indices = { 0, 1, 2, 2, 3, 0 };
}

bool i3D_vkMeshObject::loadObjFromFile(const char* objFilePath)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objFilePath))
	{
		fprintf(stderr, "VULKAN ERROR: Failed to load the Wavefront .obj model file at path \"%s\". Have you ensured that the provided model file path is correct?\n", objFilePath);
		return false;
	}

	std::unordered_map<i3D_vkVertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			i3D_vkVertex vertex = {};

			int startVertexIdx = 3 * index.vertex_index;
			int startNormalIdx = 3 * index.normal_index;
			int startTexCoordIdx = 2 * index.texcoord_index;

			vertex.vertexPosition =
			{
				attrib.vertices[startVertexIdx],
				attrib.vertices[startVertexIdx + 1],
				attrib.vertices[startVertexIdx + 2]
			};

			vertex.vertexNormal =
			{
				attrib.normals[startNormalIdx],
				attrib.normals[startNormalIdx + 1],
				attrib.normals[startNormalIdx + 2]
			};

			vertex.textureCoord =
			{
				attrib.texcoords[startTexCoordIdx],
				1.0f - attrib.texcoords[startTexCoordIdx + 1],
			};

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

bool i3D_vkMeshObject::initVertexIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue)
{
	vtxBufferSize = sizeof(vertices[0]) * vertices.size();
	
	VkDeviceSize idxBufferSize = sizeof(indices[0]) * indices.size();
	VkDeviceSize combinedBufferSize = vtxBufferSize + idxBufferSize;

	i3D_vkBufferObject stagingBuffer;

	if (!stagingBuffer.initialize
	(
		device,
		physDevice,
		combinedBufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	))
	{
		fprintf(stderr, "VULKAN ERROR: Failed to create the staging buffer for the combined vertex-index buffer.\n");
		return false;
	}

	void* data;
	if (!stagingBuffer.mapBufferMemory(device, 0, combinedBufferSize, 0, &data))
	{
		fprintf(stderr, "VULKAN ERROR: Failed to map the staging buffer memory into application address space.\n");
		stagingBuffer.cleanup(device);
		return false;
	}
	
	memcpy(data, vertices.data(), vtxBufferSize);
	memcpy(static_cast<char*>(data) + vtxBufferSize, indices.data(), idxBufferSize);

	stagingBuffer.unmapBufferMemory(device);

	if (!vtxIdxBuffer.initialize
	(
		device,
		physDevice,
		combinedBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	))
	{
		fprintf(stderr, "VULKAN ERROR: Failed to create the combined vertex-index buffer.\n");
		stagingBuffer.cleanup(device);
		return false;
	}

	if (!vtxIdxBuffer.cmdCopyBuffer(device, cmdPool, gfxQueue, stagingBuffer.buffer, 0, 0, combinedBufferSize))
	{
		fprintf(stderr, "VULKAN ERROR: Failed to copy the staging buffer memory to the combined vertex-index buffer.\n");
		stagingBuffer.cleanup(device);
		return false;
	}

	stagingBuffer.cleanup(device);
	return true;
}

bool i3D_vkMeshObject::initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* meshFilePath)
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

void i3D_vkMeshObject::draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet)
{
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(buffer, 0, 1, &vtxIdxBuffer.buffer, offsets);
	vkCmdBindIndexBuffer(buffer, vtxIdxBuffer.buffer, vtxBufferSize, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	vkCmdDrawIndexed(buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void i3D_vkMeshObject::cleanup(VkDevice device)
{	
	vtxIdxBuffer.cleanup(device);
}