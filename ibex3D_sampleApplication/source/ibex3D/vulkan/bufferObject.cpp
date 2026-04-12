#include <ibex3D/vulkan/bufferObject.h>
#include <ibex3D/vulkan/utils.h>

// ----------------------------------------------------------------------------------------------------

bool vkBufferObject::initialize(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkBufferObject::initialize()", "Couldn't create the buffer.");
		return false;
	}

	uint32_t memoryType = 0;
	VkMemoryRequirements memRequirements = {};
	
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	if (!vkUtils::findMemoryType(physDevice, memRequirements.memoryTypeBits, memProps, memoryType))
	{
		vkUtils::printVkResultError(result, "vkBufferObject::initialize()", "Couldn't find a suitable type for the buffer memory.");
		return false;
	}

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = memoryType;

	result = vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkBufferObject::initialize()", "Couldn't allocate the buffer memory.");
		return false;
	}

	result = vkBindBufferMemory(device, buffer, bufferMemory, 0);

	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkBufferObject::initialize()", "Couldn't bind the buffer memory.");
		return false;
	}

	return true;
}

void vkBufferObject::cleanup(VkDevice device)
{
	if (buffer != nullptr)
	{
		vkDestroyBuffer(device, buffer, nullptr);
		buffer = nullptr;
	}

	if (bufferMemory != nullptr)
	{
		vkFreeMemory(device, bufferMemory, nullptr);
		bufferMemory = nullptr;
	}
}

bool vkBufferObject::updateBufferData(VkDevice device, VkDeviceSize size, void* newData)
{
	void* data;
	
	VkResult result = vkMapMemory(device, bufferMemory, 0, size, 0, &data);
	
	if (result != VK_SUCCESS)
	{
		vkUtils::printVkResultError(result, "vkBufferObject::updateBufferData()", "Couldn't map the buffer memory.");
		return false;
	}
	
	memcpy(data, newData, size);

	vkUnmapMemory(device, bufferMemory);

	return true;
}

bool vkBufferObject::cmdCopyBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue gfxQueue, VkDeviceSize size, VkBuffer srcBuffer)
{
	VkCommandBuffer commandBuffer = vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (commandBuffer == nullptr)
	{
		vkUtils::printVkError("vkBufferObject::cmdCopyBuffer()", "Couldn't begin the single-time commands.");
		return false;
	}

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, buffer, 1, &copyRegion);

	vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, commandBuffer);
	
	return true;
}
