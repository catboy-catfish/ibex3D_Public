#pragma once

#include <vulkan/vulkan.h>

#include <vector>

// ----------------------------------------------------------------------------------------------------

struct i3D_vkDescriptorLayoutBuilder
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	// ----------------------------------------------------------------------------------------------------

	void addBinding(uint32_t binding, VkDescriptorType descType, uint32_t descCount, VkShaderStageFlags stageFlags, const VkSampler* pImmutableSamplers);
	void clearBindings();

	VkDescriptorSetLayout buildLayout(VkDevice device, VkDescriptorSetLayoutCreateFlags flags, void* pNext);
};

/*
struct i3D_vkPoolSizeRatio
{
	VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	float ratio = 0.0f;
};
*/

struct i3D_vkDescriptorAllocator
{
	VkDescriptorPool descriptorPool = nullptr;

	std::vector<VkDescriptorPoolSize> poolSizes;
	std::vector<VkDescriptorSet> descriptorSets;

	// ----------------------------------------------------------------------------------------------------

	void addPoolSize(VkDescriptorType descType, uint32_t descCount);
	void clearPoolSizes();

	bool initPool(VkDevice device, uint32_t maxSets, /*const std::vector<i3D_vkPoolSizeRatio>& poolSizeRatios, */ VkDescriptorPoolCreateFlags flags, void* pNext);
	bool resetPool(VkDevice device);
	void cleanupPool(VkDevice device);

	bool allocateSets(VkDevice device, uint32_t setCount, VkDescriptorSetLayout* pLayouts, void* pNext);
	void clearSets();
};