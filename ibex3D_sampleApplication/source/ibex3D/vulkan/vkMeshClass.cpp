#include <ibex3D/vulkan/vkMeshClass.h>
#include <ibex3D/vulkan/vkDefinitions.h>
#include <ibex3D/vulkan/vkTextureClass.h>
#include <ibex3D/vulkan/vkUtils.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
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
			glm::vec3 pos = glm::vec3(vertex.position.x, vertex.position.y, vertex.position.z);
			glm::vec3 col = glm::vec3(vertex.color.x, vertex.color.y, vertex.color.z);
			glm::vec2 txc = glm::vec2(vertex.texCoord.x, vertex.texCoord.y);

			// The ^ is the bitwise XOR operator
			// TODO: Look into hashing to better understand this bullshit

			return	((hash<glm::vec3>()(pos) ^ 
					(hash<glm::vec3>()(col) << 1)) >> 1) ^ 
					(hash<glm::vec2>()(txc) << 1);
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

void vkMeshClass::initSimpleModel()
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

bool vkMeshClass::loadObjFromFile(const char* objFilePath)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objFilePath))
	{
		vkUtils::printVkError("vkMeshClass::loadObjFromFile()", "Couldn't load the model file.");
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

bool vkMeshClass::initVertexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue)
{
	VkDeviceSize bufferSize = vertices.size() * sizeof(vertices[0]);

	VkBuffer stagingBuffer = nullptr;
	VkDeviceMemory stagingBufferMemory = nullptr;

	bool result = vkUtils::createBuffer
	(
		device,
		physDevice,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	if (!result)
	{
		vkUtils::printVkError("vkMeshClass::initVertexBuffer()", "Couldn't create the staging buffer.");
		return false;
	}

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, stagingBufferMemory);

	result = vkUtils::createBuffer
	(
		device,
		physDevice,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vtxBuffer,
		vtxBufferMemory
	);

	if (!result)
	{
		vkUtils::printVkError("vkMeshClass::initVertexBuffer()", "Couldn't create the vertex buffer.");
		vkUtils::destroyBuffer(device, stagingBuffer, stagingBufferMemory);
		return false;
	}

	result = vkUtils::copyBuffer
	(
		device,
		cmdPool,
		gfxQueue,
		stagingBuffer,
		vtxBuffer,
		bufferSize
	);

	if (!result)
	{
		vkUtils::printVkError("vkMeshClass::initVertexBuffer()", "Couldn't copy the staging buffer memory to the vertex buffer.");
		vkUtils::destroyBuffer(device, stagingBuffer, stagingBufferMemory);
		return false;
	}

	vkUtils::destroyBuffer(device, stagingBuffer, stagingBufferMemory);
	return true;
}

bool vkMeshClass::initIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue)
{
	VkDeviceSize bufferSize = indices.size() * sizeof(indices[0]);

	VkBuffer stagingBuffer = nullptr;
	VkDeviceMemory stagingBufferMemory = nullptr;

	bool result = vkUtils::createBuffer
	(
		device,
		physDevice,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	if (!result)
	{
		vkUtils::printVkError("vkMeshClass::initIndexBuffer()", "Couldn't create the staging buffer.");
		return false;
	}

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, stagingBufferMemory);

	result = vkUtils::createBuffer
	(
		device,
		physDevice,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		idxBuffer,
		idxBufferMemory
	);

	if (!result)
	{
		vkUtils::printVkError("vkMeshClass::initIndexBuffer()", "Couldn't create the index buffer.");
		vkUtils::destroyBuffer(device, stagingBuffer, stagingBufferMemory);
		return false;
	}

	result = vkUtils::copyBuffer
	(
		device,
		cmdPool,
		gfxQueue,
		stagingBuffer,
		idxBuffer,
		bufferSize
	);

	if (!result)
	{
		vkUtils::printVkError("vkMeshClass::initVertexBuffer()", "Couldn't copy the staging buffer memory to the index buffer.");
		vkUtils::destroyBuffer(device, stagingBuffer, stagingBufferMemory);
		return false;
	}

	vkUtils::destroyBuffer(device, stagingBuffer, stagingBufferMemory);
	return true;
}

bool vkMeshClass::initUniformBuffers(VkDevice device, VkPhysicalDevice physDevice)
{
	VkDeviceSize bufferSize = sizeof(vkUniformBufferData);

	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (!vkUtils::createBuffer
		(
			device,
			physDevice,
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffers[i],
			uniformBuffersMemory[i]
		))
		{
			return false;
		}

		vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
		// TODO: Add error checking?
	}
	
	return true;
}

void vkMeshClass::updateUniformBuffer(uint32_t currentImage, const VkExtent2D& swapchainExtent)
{
	vkUniformBufferData data = {};
	data.modelMatrix = glm::rotate(glm::mat4(1.0f), currentRotation, glm::vec3(0.0f, 0.0f, 1.0f));
	data.viewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	data.projMatrix = glm::perspective(glm::radians(45.0f), swapchainExtent.width / (float)swapchainExtent.height, 0.1f, 10.0f);
	data.projMatrix[1][1] *= -1.0f;

	memcpy(uniformBuffersMapped[currentImage], &data, sizeof(data));
}

bool vkMeshClass::initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* meshFilePath, vkTextureClass* texture)
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

	if (!initUniformBuffers(device, physDevice))
	{
		return false;
	}

	return true;
}

void vkMeshClass::setMeshRotation(float rotation)
{
	currentRotation = rotation;
}

void vkMeshClass::draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet)
{
	VkBuffer vertexBuffers[] = { vtxBuffer };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(buffer, idxBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	vkCmdDrawIndexed(buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void vkMeshClass::cleanup(VkDevice device)
{	
	for (auto& buffer : uniformBuffers)
	{
		vkDestroyBuffer(device, buffer, nullptr);
		buffer = nullptr;
	}

	uniformBuffers.clear();

	for (auto& memory : uniformBuffersMemory)
	{
		vkUnmapMemory(device, memory);
		vkFreeMemory(device, memory, nullptr);
		memory = nullptr;
	}

	uniformBuffersMemory.clear();

	vkUtils::destroyBuffer(device, idxBuffer, idxBufferMemory);
	vkUtils::destroyBuffer(device, vtxBuffer, vtxBufferMemory);
}