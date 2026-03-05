#include <ibexVulkan/vkVertexBufferClass.h>
#include <ibexVulkan/vkUtils.h>

static bool findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t& outputMemoryType)
{
	VkPhysicalDeviceMemoryProperties memProperties = {};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
	
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && ((memProperties.memoryTypes[i].propertyFlags & properties) == properties))
		{
			outputMemoryType = i;
			return true;
		}
	}

	vkUtils::printVkError("findMemoryType()", "Couldn't find any suitable memory type.");
	return false;
}

#pragma region vkVertex
VkVertexInputBindingDescription vkVertex::getBindingDesc()
{
	VkVertexInputBindingDescription bindingDesc = {};

	bindingDesc.binding = 0;
	bindingDesc.stride = sizeof(vkVertex);
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDesc;
}

std::array<VkVertexInputAttributeDescription, 2> vkVertex::getAttribDescs()
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
#pragma endregion

#pragma region vkVertexBufferClass
void vkVertexBufferClass::initVertices()
{
	vertices.clear();
	
	//					 Position		 Color
	vertices.push_back({ { 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f} });
	vertices.push_back({ { 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f} });
	vertices.push_back({ {-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f} });
}

bool vkVertexBufferClass::initialize(VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
{
#pragma region Vertex buffer
	initVertices();
	
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(vertices[0]) * vertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &vtxBuffer);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkVertexBufferClass::initialize()", "Couldn't create the vertex buffer.");
		return false;
	}
#pragma endregion

#pragma region Vertex buffer memory
	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(logicalDevice, vtxBuffer, &memRequirements);

	uint32_t memoryType = 0;
	if (!findMemoryType
	(
		physicalDevice,
		memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		memoryType
	))
	{
		return false;
	}

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = memoryType;

	result = vkAllocateMemory(logicalDevice, &allocateInfo, nullptr, &vtxBufferMemory);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkVertexBufferClass::initialize()", "Couldn't allocate the vertex buffer memory.");
		return false;
	}

	vkBindBufferMemory(logicalDevice, vtxBuffer, vtxBufferMemory, 0);

	void* data;
	vkMapMemory(logicalDevice, vtxBufferMemory, 0, bufferInfo.size, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferInfo.size));
	vkUnmapMemory(logicalDevice, vtxBufferMemory);
#pragma endregion

	return true;
}

void vkVertexBufferClass::cleanup(VkDevice logicalDevice)
{
	if (vtxBuffer != nullptr)
	{
		vkDestroyBuffer(logicalDevice, vtxBuffer, nullptr);
		vtxBuffer = nullptr;
	}

	if (vtxBufferMemory != nullptr)
	{
		vkFreeMemory(logicalDevice, vtxBufferMemory, nullptr);
		vtxBufferMemory = nullptr;
	}
}
#pragma endregion