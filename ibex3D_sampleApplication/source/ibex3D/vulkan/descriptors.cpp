#include <ibex3D/vulkan/descriptors.h>

#include <ibex3D/core/logger.h>

#include <vulkan/vk_enum_string_helper.h>

void i3D_vkDescriptorLayoutBuilder::addBinding(uint32_t binding, VkDescriptorType type)
{
	VkDescriptorSetLayoutBinding newBinding = {};
	newBinding.binding = binding;
	newBinding.descriptorCount = 1;
	newBinding.descriptorType = type;

	bindings.push_back(newBinding);
}

void i3D_vkDescriptorLayoutBuilder::clearAllBindings()
{
	bindings.clear();
}

VkDescriptorSetLayout i3D_vkDescriptorLayoutBuilder::buildLayout(VkDevice device, VkShaderStageFlags shaderStages, VkDescriptorSetLayoutCreateFlags flags, void* pNext)
{
	for (auto& b : bindings)
	{
		b.stageFlags |= shaderStages;
	}

	VkDescriptorSetLayoutCreateInfo layoutCI = {};
	layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCI.bindingCount = static_cast<uint32_t>(bindings.size());
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

bool i3D_vkDescriptorAllocator::initDescriptorPool(VkDevice device, uint32_t maxSets, const std::vector<i3D_vkPoolSizeRatio>& poolSizeRatios)
{
	std::vector<VkDescriptorPoolSize> poolSizes;

	for (i3D_vkPoolSizeRatio ratio : poolSizeRatios)
	{
		VkDescriptorPoolSize size = {};
		size.descriptorCount = static_cast<uint32_t>(ratio.ratio * maxSets);
		size.type = ratio.descriptorType;
		
		poolSizes.push_back(size);
	}

	VkDescriptorPoolCreateInfo poolCI = {};
	poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCI.maxSets = maxSets;
	poolCI.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolCI.pPoolSizes = poolSizes.data();
	
	VkResult result = vkCreateDescriptorPool(device, &poolCI, nullptr, &descriptorPool);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the descriptor pool. VkResult: %s\n", string_VkResult(result));
		return false;
	}

	return true;
}

bool i3D_vkDescriptorAllocator::resetDescriptorPool(VkDevice device)
{
	VkResult result = vkResetDescriptorPool(device, descriptorPool, 0);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to reset the descriptor pool. VkResult: %s\n", string_VkResult(result));
		return false;
	}
	
	return true;
}

void i3D_vkDescriptorAllocator::cleanupDescriptorPool(VkDevice device)
{
	if (descriptorPool != nullptr)
	{
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		descriptorPool = nullptr;
	}
}

VkDescriptorSet i3D_vkDescriptorAllocator::allocateSet(VkDevice device, VkDescriptorSetLayout* pLayout)
{
	VkDescriptorSetAllocateInfo descriptorSetAI = {};
	descriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAI.descriptorPool = descriptorPool;
	descriptorSetAI.descriptorSetCount = 1;
	descriptorSetAI.pSetLayouts = pLayout;
	
	VkDescriptorSet set = nullptr;

	VkResult result = vkAllocateDescriptorSets(device, &descriptorSetAI, &set);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to allocate the descriptor set. VkResult: %s\n", string_VkResult(result));
		return nullptr;
	}

	return set;
}