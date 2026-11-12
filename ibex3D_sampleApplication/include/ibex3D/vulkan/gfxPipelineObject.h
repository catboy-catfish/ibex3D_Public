#pragma once

#include <vulkan/vulkan.h>

struct i3D_gfxPipelineObject
{
	VkPipelineLayout layout = nullptr;
	VkPipeline pipeline = nullptr;

	bool initLayout(VkDevice device, VkPipelineLayoutCreateInfo* pCreateInfo);
	bool initPipeline(VkDevice device, VkGraphicsPipelineCreateInfo* pCreateInfo);

	void cleanup(VkDevice device);
};