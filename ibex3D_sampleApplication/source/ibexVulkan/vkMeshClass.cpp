#include <ibexVulkan/vkMeshClass.h>
#include <ibexVulkan/vkUtils.h>

VkVertexInputBindingDescription vkVertex::getBindingDesc()
{
	VkVertexInputBindingDescription bindingDesc = {};

	bindingDesc.binding = 0;
	bindingDesc.stride = sizeof(vkVertex);
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDesc;
}

std::array<VkVertexInputAttributeDescription, 2> vkVertex::getAttributeDescs()
{
	std::array<VkVertexInputAttributeDescription, 2> attribDescs = {};
	
	// Position
	attribDescs[0].binding = 0;
	attribDescs[0].location = 0;
	attribDescs[0].format = VK_FORMAT_R32G32_SFLOAT;
	attribDescs[0].offset = offsetof(vkVertex, position);

	// Color
	attribDescs[1].binding = 0;
	attribDescs[1].location = 1;
	attribDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDescs[1].offset = offsetof(vkVertex, color);

	return attribDescs;
}

void vkMeshClass::initMeshData()
{
	vertices =
	{
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}
	};

	indices =
	{
		0, 1, 2, 2, 3, 0
	};
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

bool vkMeshClass::initialize(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	initMeshData();
	
	if (!initVertexBuffer(physicalDevice, logicalDevice, commandPool, graphicsQueue))
	{
		return false;
	}

	if (!initIndexBuffer(physicalDevice, logicalDevice, commandPool, graphicsQueue))
	{
		return false;
	}
	
	return true;
}

void vkMeshClass::cleanup(VkDevice logicalDevice)
{
	vkUtils::destroyBuffer(logicalDevice, idxBuffer, idxBufferMemory);
	vkUtils::destroyBuffer(logicalDevice, vtxBuffer, vtxBufferMemory);
}