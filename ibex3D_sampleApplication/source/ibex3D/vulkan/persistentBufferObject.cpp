#include <ibex3D/vulkan/persistentBufferObject.h>
#include <ibex3D/vulkan/utils.h>

#include <ibex3D/utility/logger.h>

// ----------------------------------------------------------------------------------------------------

bool vkPersistentBufferObject::initialize(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);

	if (result != VK_SUCCESS)
	{
		vkUtils::logErrorWithResult(result, "vkPersistentBufferObject::initialize(): An error occurred while trying to create the buffer.", __FILE__, __LINE__ - 4);
		return false;
	}

	uint32_t memoryType = 0;
	VkMemoryRequirements memRequirements = {};

	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	if (!vkUtils::findMemoryType(physDevice, memRequirements.memoryTypeBits, memProps, memoryType))
	{
		logger::logError("vkPersistentBufferObject::initialize(): Couldn't find a suitable type for the buffer memory.", __FILE__, __LINE__ - 4);
		return false;
	}

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = memoryType;

	result = vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory);

	if (result != VK_SUCCESS)
	{
		vkUtils::logErrorWithResult(result, "vkPersistentBufferObject::initialize(): An error occured while trying to allocate the buffer memory.", __FILE__, __LINE__ - 4);
		return false;
	}

	result = vkBindBufferMemory(device, buffer, bufferMemory, 0);

	if (result != VK_SUCCESS)
	{
		vkUtils::logErrorWithResult(result, "vkPersistentBufferObject::initialize(): An error occured while trying to bind the buffer memory.", __FILE__, __LINE__ - 4);
		return false;
	}

	return true;
}

void vkPersistentBufferObject::cleanup(VkDevice device)
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

bool vkPersistentBufferObject::mapBufferData(VkDevice device, VkDeviceSize size)
{
	VkResult result = vkMapMemory(device, bufferMemory, 0, size, 0, &bufferData);
	
	if (result != VK_SUCCESS)
	{
		vkUtils::logErrorWithResult(result, "vkPersistentBufferObject::mapBufferData(): An error occured while trying to map the buffer memory.", __FILE__, __LINE__ - 4);
		return false;
	}

	return true;
}

void vkPersistentBufferObject::setBufferData(VkDeviceSize size, void* newData)
{
	memcpy(bufferData, newData, size);
}

void vkPersistentBufferObject::unmapBufferData(VkDevice device)
{
	vkUnmapMemory(device, bufferMemory);
	bufferData = nullptr;
}

bool vkPersistentBufferObject::cmdCopyBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue gfxQueue, VkDeviceSize size, VkBuffer srcBuffer)
{
	VkCommandBuffer commandBuffer = vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (commandBuffer == nullptr)
	{
		logger::logError("vkPersistentBufferObject::cmdCopyBuffer(): Couldn't begin the single-time commands.", __FILE__, __LINE__ - 4);
		return false;
	}

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, buffer, 1, &copyRegion);

	vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, commandBuffer);

	return true;
}