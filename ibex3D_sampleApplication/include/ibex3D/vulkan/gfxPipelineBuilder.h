#pragma once

#include <vulkan/vulkan.h>
#include <vector>

// ----------------------------------------------------------------------------------------------------

struct i3D_vkGfxPipelineBuilder
{
	VkViewport viewport = {};
	VkRect2D scissor = {};
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};

	VkPipelineDynamicStateCreateInfo dynamicStateCI = {};
	VkPipelineVertexInputStateCreateInfo vertexInputStateCI = {};
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = {};
	VkPipelineViewportStateCreateInfo viewportStateCI = {};
	VkPipelineRasterizationStateCreateInfo rasterStateCI = {};
	VkPipelineMultisampleStateCreateInfo multisampleStateCI = {};
	VkPipelineColorBlendStateCreateInfo colorBlendStateCI = {};
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = {};

	void clearEverything();

	void initDynamicState(const uint32_t& dynStateCount, VkDynamicState* pDynStates);
	void initVertexInputState(const uint32_t& bindDescCount, VkVertexInputBindingDescription* pBindDescs, const uint32_t& attribDescCount, VkVertexInputAttributeDescription* pAttribDescs);
	void initInputAssemblyState(const VkPrimitiveTopology& primitiveTopology);
	void initViewportState(const VkExtent2D& extent);
	void initRasterState(const VkPolygonMode& polygonMode, const VkCullModeFlags& cullMode, const VkFrontFace& frontFace);
	void initMultisampleState_enabled(const VkSampleCountFlagBits& sampleCount, const VkBool32& useSampleShading, const VkBool32& useAtoC, const VkBool32& useAtoOne);
	void initMultisampleState_disabled();
	void initColorBlendState_disabled();
	void initDepthStencilState_enabled(const VkBool32& useDepthTest, const VkCompareOp& compareOp);

	VkPipelineLayout buildPipelineLayout(VkDevice device, const uint32_t& setLayoutCount, VkDescriptorSetLayout* pSetLayouts, const uint32_t& pushConstantRangeCount, VkPushConstantRange* pPushConstantRanges);
	VkPipeline buildGraphicsPipeline(VkDevice device, const uint32_t& stageCount, VkPipelineShaderStageCreateInfo* pStages, VkPipelineLayout layout, VkRenderPass renderPass);
};