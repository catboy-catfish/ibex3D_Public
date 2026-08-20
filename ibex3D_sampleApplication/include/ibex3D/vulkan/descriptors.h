#pragma once

#include <vulkan/vulkan.h>

#include <vector>

// ----------------------------------------------------------------------------------------------------

struct i3D_vkDescriptorLayoutBuilder
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	// ----------------------------------------------------------------------------------------------------

	void addBinding(uint32_t binding, VkDescriptorType type);
	void clearAllBindings();

	VkDescriptorSetLayout buildLayout(VkDevice device, VkShaderStageFlags shaderStages, VkDescriptorSetLayoutCreateFlags flags, void* pNext);
};

struct i3D_vkPoolSizeRatio
{
	VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	float ratio = 0.0f;
};

struct i3D_vkDescriptorAllocator
{
	VkDescriptorPool descriptorPool = nullptr;

	// ----------------------------------------------------------------------------------------------------

	bool initDescriptorPool(VkDevice device, uint32_t maxSets, const std::vector<i3D_vkPoolSizeRatio>& poolSizeRatios);
	bool resetDescriptorPool(VkDevice device);
	void cleanupDescriptorPool(VkDevice device);

	VkDescriptorSet allocateSet(VkDevice device, VkDescriptorSetLayout* pLayout);
};