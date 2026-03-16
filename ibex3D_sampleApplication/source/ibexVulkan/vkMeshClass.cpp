#include <ibexVulkan/vkMeshClass.h>
#include <ibexVulkan/vkUtils.h>

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
			// TODO: Look into hashing to understand this bullshit

			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

bool vkVertex::operator ==(const vkVertex& other) const
{
	return (position == other.position) && (color == other.color) && (texCoord == other.texCoord);
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

bool vkMeshClass::initTexture(VkDevice device, VkPhysicalDevice physDevice, const char* textureFilePath, VkCommandPool cmdPool, VkQueue gfxQueue)
{
	if (!textureClass.initialize(device, physDevice, textureFilePath, cmdPool, gfxQueue))
	{
		return false;
	}
	
	return true;
}

bool vkMeshClass::initModel(const char* meshFilePath)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, meshFilePath))
	{
		vkUtils::printVkError("vkMeshClass::initModel()", "Couldn't load the model file.\n");
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

bool vkMeshClass::initVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	VkDeviceSize bufferSize = vertices.size() * sizeof(vertices[0]);

	VkBuffer stagingBuffer = nullptr;
	VkDeviceMemory stagingBufferMemory = nullptr;

	bool result = vkUtils::createBuffer
	(
		physicalDevice,
		logicalDevice,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	if (!result)
	{
		vkUtils::printVkError("vkMeshClass::initVertexBuffer()", "Couldn't create the staging buffer.\n");
		return false;
	}

	void* data;
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(logicalDevice, stagingBufferMemory);

	result = vkUtils::createBuffer
	(
		physicalDevice,
		logicalDevice,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vtxBuffer,
		vtxBufferMemory
	);

	if (!result)
	{
		vkUtils::printVkError("vkMeshClass::initVertexBuffer()", "Couldn't create the vertex buffer.\n");
		vkUtils::destroyBuffer(logicalDevice, stagingBuffer, stagingBufferMemory);
		return false;
	}

	result = vkUtils::copyBuffer
	(
		logicalDevice,
		commandPool,
		graphicsQueue,
		stagingBuffer,
		vtxBuffer,
		bufferSize
	);

	if (!result)
	{
		vkUtils::printVkError("vkMeshClass::initVertexBuffer()", "Couldn't copy the staging buffer memory to the vertex buffer.\n");
		vkUtils::destroyBuffer(logicalDevice, stagingBuffer, stagingBufferMemory);
		return false;
	}

	vkUtils::destroyBuffer(logicalDevice, stagingBuffer, stagingBufferMemory);
	return true;
}

bool vkMeshClass::initIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	VkDeviceSize bufferSize = indices.size() * sizeof(indices[0]);

	VkBuffer stagingBuffer = nullptr;
	VkDeviceMemory stagingBufferMemory = nullptr;

	bool result = vkUtils::createBuffer
	(
		physicalDevice,
		logicalDevice,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	if (!result)
	{
		vkUtils::printVkError("vkMeshClass::initIndexBuffer()", "Couldn't create the staging buffer.\n");
		return false;
	}

	void* data;
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(logicalDevice, stagingBufferMemory);

	result = vkUtils::createBuffer
	(
		physicalDevice,
		logicalDevice,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		idxBuffer,
		idxBufferMemory
	);

	if (!result)
	{
		vkUtils::printVkError("vkMeshClass::initIndexBuffer()", "Couldn't create the index buffer.\n");
		vkUtils::destroyBuffer(logicalDevice, stagingBuffer, stagingBufferMemory);
		return false;
	}

	result = vkUtils::copyBuffer
	(
		logicalDevice,
		commandPool,
		graphicsQueue,
		stagingBuffer,
		idxBuffer,
		bufferSize
	);

	if (!result)
	{
		vkUtils::printVkError("vkMeshClass::initVertexBuffer()", "Couldn't copy the staging buffer memory to the index buffer.\n");
		vkUtils::destroyBuffer(logicalDevice, stagingBuffer, stagingBufferMemory);
		return false;
	}

	vkUtils::destroyBuffer(logicalDevice, stagingBuffer, stagingBufferMemory);
	return true;
}

bool vkMeshClass::initUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, size_t maxFramesInFlight)
{
	VkDeviceSize bufferSize = sizeof(vkUniformBufferData);

	uniBuffers.resize(maxFramesInFlight);
	uniBuffersMemory.resize(maxFramesInFlight);
	uniBuffersMapped.resize(maxFramesInFlight);

	for (size_t i = 0; i < maxFramesInFlight; i++)
	{
		if (!vkUtils::createBuffer
		(
			physicalDevice,
			logicalDevice,
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniBuffers[i],
			uniBuffersMemory[i]
		))
		{
			return false;
		}

		vkMapMemory(logicalDevice, uniBuffersMemory[i], 0, bufferSize, 0, &uniBuffersMapped[i]);
		// TODO: Add error checking?
	}
	
	return true;
}

bool vkMeshClass::initDescriptorSetLayout(VkDevice logicalDevice)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings =
	{
		uboLayoutBinding, samplerLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkResult result = vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkMeshClass::initDescriptorSetLayout()", "Couldn't create the descriptor set layout for the uniform buffer.");
		return false;
	}
	
	return true;
}

bool vkMeshClass::initDescriptorPoolAndSets(VkDevice logicalDevice, VkImageView textureImageView, VkSampler textureSampler, size_t maxFramesInFlight)
{	
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(maxFramesInFlight);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = poolSizes[0].descriptorCount;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(maxFramesInFlight);

	VkResult result = vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkMeshClass::initDescriptorPoolAndSets()", "Couldn't create the descriptor pool.");
		return false;
	}

	// ----------------------------------------------------------------------------------------------------

	std::vector<VkDescriptorSetLayout> layouts(maxFramesInFlight, descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(maxFramesInFlight);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(maxFramesInFlight);

	result = vkAllocateDescriptorSets(logicalDevice, &allocInfo, descriptorSets.data());

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkMeshClass::initDescriptorPoolAndSets()", "Couldn't allocate the descriptor sets.");
		return false;
	}

	for (size_t i = 0; i < maxFramesInFlight; i++)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(vkUniformBufferData);
		
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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

	memcpy(uniBuffersMapped[currentImage], &data, sizeof(data));
}

bool vkMeshClass::initialize(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, size_t maxFramesInFlight)
{
	const char* meshFilePath = "assets/models/viking_room.obj";
	const char* textureFilePath = "assets/images/viking_room.png";
	
	if (!initTexture(logicalDevice, physicalDevice, textureFilePath, commandPool, graphicsQueue))
	{
		return false;
	}
	
	if (!initModel(meshFilePath))
	{
		return false;
	}
	
	if (!initVertexBuffer(physicalDevice, logicalDevice, commandPool, graphicsQueue))
	{
		return false;
	}

	if (!initIndexBuffer(physicalDevice, logicalDevice, commandPool, graphicsQueue))
	{
		return false;
	}

	if (!initUniformBuffers(physicalDevice, logicalDevice, maxFramesInFlight))
	{
		return false;
	}

	if (!initDescriptorPoolAndSets(logicalDevice, textureClass.imageView, textureClass.sampler, maxFramesInFlight))
	{
		return false;
	}

	return true;
}

void vkMeshClass::setMeshRotation(float rotation)
{
	currentRotation = rotation;
}

void vkMeshClass::draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
	VkBuffer vertexBuffers[] = { vtxBuffer };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(buffer, idxBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

	vkCmdDrawIndexed(buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void vkMeshClass::cleanup(VkDevice logicalDevice)
{	
	for (auto& buffer : uniBuffers)
	{
		vkDestroyBuffer(logicalDevice, buffer, nullptr);
		buffer = nullptr;
	}

	uniBuffers.clear();

	for (auto& memory : uniBuffersMemory)
	{
		vkUnmapMemory(logicalDevice, memory);
		vkFreeMemory(logicalDevice, memory, nullptr);
		memory = nullptr;
	}

	uniBuffersMemory.clear();
	
	if (descriptorPool != nullptr)
	{
		vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
		descriptorPool = nullptr;
	}

	if (descriptorSetLayout != nullptr)
	{
		vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
		descriptorSetLayout = nullptr;
	}

	vkUtils::destroyBuffer(logicalDevice, idxBuffer, idxBufferMemory);
	vkUtils::destroyBuffer(logicalDevice, vtxBuffer, vtxBufferMemory);

	textureClass.cleanup(logicalDevice);
}