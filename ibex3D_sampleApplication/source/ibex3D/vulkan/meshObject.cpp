#include <ibex3D/vulkan/meshObject.h>
#include <ibex3D/vulkan/meshUtils.h>
#include <ibex3D/vulkan/utils.h>

#include <ibex3D/core/logger.h>

#include <unordered_map>

// ----------------------------------------------------------------------------------------------------

bool i3D_vkMeshObject::initVertexIndexBuffer(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* meshFilePath)
{
	i3D_staticMeshLoader loader;

	if (!loader.loadMeshFromObj(meshFilePath))
	{
		return false;
	}

	indexCount = static_cast<uint32_t>(loader.getIndexCount());
	
	vtxBufferSize = sizeof(loader.vertices[0]) * loader.vertices.size();
	
	VkDeviceSize idxBufferSize = sizeof(loader.indices[0]) * loader.indices.size();
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
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the staging buffer for the combined vertex-index buffer.\n");
		return false;
	}

	void* data;
	if (!stagingBuffer.mapBufferMemory(device, 0, combinedBufferSize, 0, &data))
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to map the staging buffer memory into application address space.\n");
		stagingBuffer.cleanup(device);
		return false;
	}
	
	memcpy(data, loader.vertices.data(), vtxBufferSize);
	memcpy(static_cast<char*>(data) + vtxBufferSize, loader.indices.data(), idxBufferSize);

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
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the combined vertex-index buffer.\n");
		stagingBuffer.cleanup(device);
		return false;
	}

	VkCommandBuffer cmdBuffer = i3D_vkUtils::beginSingleTimeCommands(device, cmdPool);

	if (!vtxIdxBuffer.cmdCopyBuffer(device, cmdBuffer, stagingBuffer.buffer, 0, 0, combinedBufferSize))
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to copy the staging buffer memory to the combined vertex-index buffer.\n");
		stagingBuffer.cleanup(device);
		return false;
	}

	i3D_vkUtils::endSingleTimeCommands(device, cmdPool, gfxQueue, cmdBuffer);

	stagingBuffer.cleanup(device);
	return true;
}

bool i3D_vkMeshObject::initialize(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool cmdPool, VkQueue gfxQueue, const char* meshFilePath)
{	
	if (!initVertexIndexBuffer(device, physDevice, cmdPool, gfxQueue, meshFilePath))
	{
		return false;
	}

	return true;
}

void i3D_vkMeshObject::draw(VkCommandBuffer buffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, VkDescriptorSet* pDescriptorSet)
{
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	vkCmdBindVertexBuffers(buffer, 0, 1, &vtxIdxBuffer.buffer, offsets);
	vkCmdBindIndexBuffer(buffer, vtxIdxBuffer.buffer, vtxBufferSize, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, pDescriptorSet, 0, nullptr);

	vkCmdDrawIndexed(buffer, indexCount, 1, 0, 0, 0);
}

void i3D_vkMeshObject::cleanup(VkDevice device)
{	
	vtxIdxBuffer.cleanup(device);
}