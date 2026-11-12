#include <ibex3D/vulkan/gfxPipelineObject.h>

bool i3D_gfxPipelineObject::initLayout(VkDevice device, VkPipelineLayoutCreateInfo* pCreateInfo)
{
	VkResult result = vkCreatePipelineLayout(device, pCreateInfo, nullptr, &layout);
	
	if (result != VK_SUCCESS)
	{
		return false;
	}

	return true;
}

bool i3D_gfxPipelineObject::initPipeline(VkDevice device, VkGraphicsPipelineCreateInfo* pCreateInfo)
{
	VkResult result = vkCreateGraphicsPipelines(device, 0, 1, pCreateInfo, nullptr, &pipeline);
	
	if (result != VK_SUCCESS)
	{
		return false;
	}

	return true;
}

void i3D_gfxPipelineObject::cleanup(VkDevice device)
{
	if (pipeline != nullptr)
	{
		vkDestroyPipeline(device, pipeline, nullptr);
		pipeline = nullptr;
	}

	if (layout != nullptr)
	{
		vkDestroyPipelineLayout(device, layout, nullptr);
		layout = nullptr;
	}
}
