#include <ibex3D/vulkan/bufferObject.h>
#include <ibex3D/vulkan/utils.h>

#include <ibex3D/utility/logger.h>

// ----------------------------------------------------------------------------------------------------

bool vkBufferObject::initialize(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);

	if (result != VK_SUCCESS)
	{
		vkUtils::logErrorWithResult(result, "vkBufferObject::initialize(): An error occured while trying to create the buffer.", __FILE__, __LINE__ - 4);
		return false;
	}

	bufferSize = size;

	uint32_t memoryType = 0;
	VkMemoryRequirements memRequirements = {};

	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	if (!vkUtils::findMemoryType(physDevice, memRequirements.memoryTypeBits, memProperties, memoryType))
	{
		logger::logError("vkBufferObject::initialize(): Couldn't find a suitable type for the buffer memory.", __FILE__, __LINE__ - 2);
		return false;
	}

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = memoryType;

	result = vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory);

	if (result != VK_SUCCESS)
	{
		vkUtils::logErrorWithResult(result, "vkBufferObject::initialize(): An error occured while trying to allocate the buffer memory.", __FILE__, __LINE__ - 4);
		return false;
	}

	result = vkBindBufferMemory(device, buffer, bufferMemory, 0);

	if (result != VK_SUCCESS)
	{
		vkUtils::logErrorWithResult(result, "vkBufferObject::initialize(): An error occured while trying to bind the buffer memory.", __FILE__, __LINE__ - 4);
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

	bufferSize = 0;
}

bool vkBufferObject::mapBufferMemory(VkDevice device, VkDeviceSize regionOffset, VkDeviceSize regionSize, VkMemoryMapFlags mapFlags, void** ppData)
{
	VkResult result = vkMapMemory(device, bufferMemory, regionOffset, regionSize, mapFlags, ppData);
	
	if (result != VK_SUCCESS)
	{
		vkUtils::logErrorWithResult(result, "vkBufferObject::mapBufferData(): An error occured while trying to map the buffer memory into application address space.", __FILE__, __LINE__ - 4);
		return false;
	}

	return true;
}

void vkBufferObject::unmapBufferMemory(VkDevice device)
{
	vkUnmapMemory(device, bufferMemory);
}

bool vkBufferObject::updateBufferOnce(VkDevice device, VkDeviceSize regionOffset, VkDeviceSize regionSize, VkMemoryMapFlags mapFlags, void* newData)
{
	void* data;
	if (!mapBufferMemory(device, regionOffset, regionSize, mapFlags, &data))
	{
		logger::logError("vkBufferObject::updateBufferOnce(): Couldn't map the buffer memory.", __FILE__, __LINE__ - 2);
		return false;
	}
	
	memcpy(data, newData, regionSize);

	unmapBufferMemory(device);
	return true;
}

bool vkBufferObject::cmdCopyBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue gfxQueue, VkBuffer srcBuffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize regionSize)
{
	VkCommandBuffer commandBuffer = vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (commandBuffer == nullptr)
	{
		logger::logError("vkBufferObject::cmdCopyBuffer(): Couldn't begin the single-time commands.", __FILE__, __LINE__ - 4);
		vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, commandBuffer);
		return false;
	}

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = srcOffset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = regionSize;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, buffer, 1, &copyRegion);

	vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, commandBuffer);
	return true;
}