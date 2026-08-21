#include <ibex3D/vulkan/descriptors.h>

#include <ibex3D/core/logger.h>

#include <vulkan/vk_enum_string_helper.h>

void i3D_vkDescriptorLayoutBuilder::addBinding(uint32_t binding, VkDescriptorType descType, uint32_t descCount, VkShaderStageFlags stageFlags, const VkSampler* pImmutableSamplers)
{
	VkDescriptorSetLayoutBinding newBinding = {};
	newBinding.binding = binding;
	newBinding.descriptorType = descType;
	newBinding.descriptorCount = descCount;
	newBinding.stageFlags = stageFlags;
	newBinding.pImmutableSamplers = pImmutableSamplers;

	bindings.push_back(newBinding);
}

void i3D_vkDescriptorLayoutBuilder::clearBindings()
{
	bindings.clear();
}

VkDescriptorSetLayout i3D_vkDescriptorLayoutBuilder::buildLayout(VkDevice device, VkDescriptorSetLayoutCreateFlags flags, void* pNext)
{
	if (bindings.empty())
	{
		i3D_logErrorMessage("VULKAN ERROR: Couldn't create the descriptor set layout because the i3D_vkDescriptorLayoutBuilder has no bindings.\n");
		return nullptr;
	}
	
	VkDescriptorSetLayoutCreateInfo layoutCI = {};
	layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCI.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutCI.pBindings = bindings.data();
	layoutCI.flags = flags;
	layoutCI.pNext = pNext;

	VkDescriptorSetLayout setLayout = nullptr;

	VkResult result = vkCreateDescriptorSetLayout(device, &layoutCI, nullptr, &setLayout);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the descriptor set layout. VkResult: %s\n", string_VkResult(result));
		return nullptr;
	}

	return setLayout;
}

void i3D_vkDescriptorAllocator::addPoolSize(VkDescriptorType descType, uint32_t descCount)
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = descType;
	poolSize.descriptorCount = descCount;

	poolSizes.push_back(poolSize);
}

void i3D_vkDescriptorAllocator::clearPoolSizes()
{
	poolSizes.clear();
}

bool i3D_vkDescriptorAllocator::initPool(VkDevice device, uint32_t maxSets, VkDescriptorPoolCreateFlags flags, void* pNext)
{
	/*
	std::vector<VkDescriptorPoolSize> poolSizes;

	for (i3D_vkPoolSizeRatio ratio : poolSizeRatios)
	{
		VkDescriptorPoolSize size = {};
		size.descriptorCount = static_cast<uint32_t>(ratio.ratio * maxSets);
		size.type = ratio.descriptorType;
		
		poolSizes.push_back(size);
	}
	*/

	if (poolSizes.empty())
	{
		i3D_logErrorMessage("VULKAN ERROR: Couldn't create the descriptor pool because the i3D_vkDescriptorAllocator has no pool sizes.\n");
		return false;
	}

	VkDescriptorPoolCreateInfo poolCI = {};
	poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCI.maxSets = maxSets;
	poolCI.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolCI.pPoolSizes = poolSizes.data();
	poolCI.flags = flags;
	poolCI.pNext = pNext;
	
	VkResult result = vkCreateDescriptorPool(device, &poolCI, nullptr, &descriptorPool);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the descriptor pool. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	return true;
}

bool i3D_vkDescriptorAllocator::resetPool(VkDevice device)
{
	VkResult result = vkResetDescriptorPool(device, descriptorPool, 0);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to reset the descriptor pool. VkResult: %s\n", string_VkResult(result));
		return false;
	}
	
	return true;
}

void i3D_vkDescriptorAllocator::cleanupPool(VkDevice device)
{
	if (descriptorPool != nullptr)
	{
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		descriptorPool = nullptr;
	}
}

bool i3D_vkDescriptorAllocator::allocateSets(VkDevice device, uint32_t setCount, VkDescriptorSetLayout* pLayouts, void* pNext)
{
	VkDescriptorSetAllocateInfo descriptorSetAI = {};
	descriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAI.descriptorPool = descriptorPool;
	descriptorSetAI.descriptorSetCount = setCount;
	descriptorSetAI.pSetLayouts = pLayouts;
	descriptorSetAI.pNext = pNext;

	descriptorSets.resize(setCount);

	VkResult result = vkAllocateDescriptorSets(device, &descriptorSetAI, descriptorSets.data());

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to allocate the descriptor sets. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	return true;
}

void i3D_vkDescriptorAllocator::clearSets()
{
	// I think descriptor sets are automatically released when their parent descriptor pool is released, so this function just clears the vector
	descriptorSets.clear();
}