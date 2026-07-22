#include <ibex3D/vulkan/bufferObject.h>
#include <ibex3D/vulkan/utils.h>

#include <stdio.h>

#include <vulkan/vk_enum_string_helper.h>

// ----------------------------------------------------------------------------------------------------

bool i3D_vkBufferObject::initialize(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);

	if (result != VK_SUCCESS)
	{
		fprintf(stderr, "VULKAN ERROR: Failed to create the buffer. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	bufferSize = size;

	uint32_t memoryType = 0;
	VkMemoryRequirements memRequirements = {};

	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	if (!i3D_vkUtils::findMemoryType(physDevice, memRequirements.memoryTypeBits, memProperties, memoryType))
	{
		fprintf(stderr, "VULKAN ERROR: Couldn't find a suitable type for the buffer memory.\n");
		return false;
	}

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = memoryType;

	result = vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory);

	if (result != VK_SUCCESS)
	{
		fprintf(stderr, "VULKAN ERROR: Failed to allocate the buffer memory. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	result = vkBindBufferMemory(device, buffer, bufferMemory, 0);

	if (result != VK_SUCCESS)
	{
		fprintf(stderr, "VULKAN ERROR: Failed to bind the buffer memory. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	return true;
}

void i3D_vkBufferObject::cleanup(VkDevice device)
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

bool i3D_vkBufferObject::mapBufferMemory(VkDevice device, VkDeviceSize regionOffset, VkDeviceSize regionSize, VkMemoryMapFlags mapFlags, void** ppData)
{
	VkResult result = vkMapMemory(device, bufferMemory, regionOffset, regionSize, mapFlags, ppData);
	
	if (result != VK_SUCCESS)
	{
		fprintf(stderr, "VULKAN ERROR: Failed to map the buffer memory into application address space. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	return true;
}

void i3D_vkBufferObject::unmapBufferMemory(VkDevice device)
{
	vkUnmapMemory(device, bufferMemory);
}

bool i3D_vkBufferObject::updateBufferOnce(VkDevice device, VkDeviceSize regionOffset, VkDeviceSize regionSize, VkMemoryMapFlags mapFlags, void* newData)
{
	void* data;
	if (!mapBufferMemory(device, regionOffset, regionSize, mapFlags, &data))
	{
		return false;
	}
	
	memcpy(data, newData, regionSize);

	unmapBufferMemory(device);
	return true;
}

bool i3D_vkBufferObject::cmdCopyBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue gfxQueue, VkBuffer srcBuffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize regionSize)
{
	VkCommandBuffer commandBuffer = i3D_vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (commandBuffer == nullptr)
	{
		fprintf(stderr, "VULKAN ERROR: Couldn't begin the single-time commands for copying the buffer.\n");
		i3D_vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, commandBuffer);
		return false;
	}

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = srcOffset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = regionSize;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, buffer, 1, &copyRegion);

	i3D_vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, commandBuffer);
	return true;
}