#pragma once

#include <vulkan/vulkan.h>

// ----------------------------------------------------------------------------------------------------

static VkGraphicsPipelineCreateInfo graphicsPipelineCI
(
	const uint32_t& stageCount,
	VkPipelineShaderStageCreateInfo* pStages,
	VkPipelineVertexInputStateCreateInfo* pVertexInputState,
	VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState,
	VkPipelineViewportStateCreateInfo* pViewportState,
	VkPipelineRasterizationStateCreateInfo* pRasterState,
	VkPipelineMultisampleStateCreateInfo* pMultisampleState,
	VkPipelineDepthStencilStateCreateInfo* pDepthStencilState,
	VkPipelineColorBlendStateCreateInfo* pColorBlendState,
	VkPipelineDynamicStateCreateInfo* pDynamicState,
	VkPipelineLayout pipelineLayout,
	VkRenderPass renderPass
)
{
	VkGraphicsPipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	
	info.stageCount = stageCount;
	info.pStages = pStages;
	info.pVertexInputState = pVertexInputState;
	info.pInputAssemblyState = pInputAssemblyState;
	info.pViewportState = pViewportState;
	info.pRasterizationState = pRasterState;
	info.pMultisampleState = pMultisampleState;
	info.pDepthStencilState = pDepthStencilState;
	info.pColorBlendState = pColorBlendState;
	info.pDynamicState = pDynamicState;
	info.layout = pipelineLayout;
	info.renderPass = renderPass;
	info.subpass = 0;
	info.basePipelineHandle = VK_NULL_HANDLE;
	info.basePipelineIndex = -1;

	return info;
}

static VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentStateCI_noBlending()
{
	VkPipelineColorBlendAttachmentState attachment = {};

	attachment.blendEnable = VK_FALSE;
	attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	attachment.colorBlendOp = VK_BLEND_OP_ADD;
	attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	attachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	return attachment;
}

static VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCI(const uint32_t& attachmentCount, VkPipelineColorBlendAttachmentState* pAttachments)
{
	VkPipelineColorBlendStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

	info.logicOpEnable = VK_FALSE;
	info.logicOp = VK_LOGIC_OP_COPY;
	info.attachmentCount = attachmentCount;
	info.pAttachments = pAttachments;
	info.blendConstants[0] = 0.0f;
	info.blendConstants[1] = 0.0f;
	info.blendConstants[2] = 0.0f;
	info.blendConstants[3] = 0.0f;

	return info;
}

static VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCI_useDepth(const VkBool32& useDepthTest, const VkCompareOp& compareOp)
{
	VkPipelineDepthStencilStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

	info.depthTestEnable = useDepthTest;
	info.depthWriteEnable = VK_TRUE;
	info.depthCompareOp = compareOp;
	info.depthBoundsTestEnable = VK_FALSE;
	info.minDepthBounds = 0.0f;
	info.maxDepthBounds = 1.0f;
	info.stencilTestEnable = VK_FALSE;
	info.front = {};
	info.back = {};

	return info;
}

static VkPipelineDynamicStateCreateInfo pipelineDynamicStateCI(const uint32_t& stateCount, VkDynamicState* pStates)
{
	VkPipelineDynamicStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

	info.dynamicStateCount = stateCount;
	info.pDynamicStates = pStates;

	return info;
}

static VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCI(const VkPrimitiveTopology& topology)
{
	VkPipelineInputAssemblyStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

	info.topology = topology;
	info.primitiveRestartEnable = VK_FALSE;

	return info;
}

static VkPipelineLayoutCreateInfo pipelineLayoutCI
(
	const uint32_t& setLayoutCount,
	VkDescriptorSetLayout* pSetLayouts,
	const uint32_t& pushConstantRangeCount,
	VkPushConstantRange* pPushConstantRanges
)
{
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	info.setLayoutCount = setLayoutCount;
	info.pSetLayouts = pSetLayouts;
	info.pushConstantRangeCount = pushConstantRangeCount;
	info.pPushConstantRanges = pPushConstantRanges;

	return info;
}

static VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCI_noMSAA()
{
	VkPipelineMultisampleStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

	info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	return info;
}

static VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCI_useMSAA(const VkSampleCountFlagBits& sampleCount, const VkBool32& useSampleShading, const VkBool32& useAtoC)
{
	VkPipelineMultisampleStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

	info.rasterizationSamples = sampleCount;
	info.sampleShadingEnable = useSampleShading;
	info.minSampleShading = 0.2f;
	info.pSampleMask = nullptr;
	info.alphaToCoverageEnable = useAtoC;

	return info;
}

static VkPipelineRasterizationStateCreateInfo pipelineRasterStateCI(const VkPolygonMode& polygonMode, const VkCullModeFlags& cullMode, const VkFrontFace& frontFace)
{
	VkPipelineRasterizationStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

	info.depthClampEnable = VK_FALSE;
	info.rasterizerDiscardEnable = false;
	info.lineWidth = 1.0f;
	info.polygonMode = polygonMode;
	info.cullMode = cullMode;
	info.frontFace = frontFace;
	info.depthBiasEnable = VK_FALSE;
	info.depthBiasClamp = 0.0f;
	info.depthBiasConstantFactor = 0.0f;
	info.depthBiasSlopeFactor = 0.0f;

	return info;
}

static VkPipelineShaderStageCreateInfo pipelineShaderStageCI(const VkShaderStageFlagBits& stage, VkShaderModule shaderModule, const char* pName)
{
	VkPipelineShaderStageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	
	info.stage = stage;
	info.module = shaderModule;
	info.pName = pName;

	return info;
}

static VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCI
(
	const uint32_t& bindingCount,
	VkVertexInputBindingDescription* pBindings,
	const uint32_t& attribCount,
	VkVertexInputAttributeDescription* pAttribs
)
{
	VkPipelineVertexInputStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	info.vertexBindingDescriptionCount = bindingCount;
	info.pVertexBindingDescriptions = pBindings;
	info.vertexAttributeDescriptionCount = attribCount;
	info.pVertexAttributeDescriptions = pAttribs;

	return info;
}

static VkPipelineViewportStateCreateInfo pipelineViewportStateCI(const uint32_t& viewportCount, VkViewport* pViewports, const uint32_t& scissorCount, VkRect2D* pScissors)
{
	VkPipelineViewportStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

	info.viewportCount = viewportCount;
	info.pViewports = pViewports;
	info.scissorCount = scissorCount;
	info.pScissors = pScissors;

	return info;
}

static VkViewport viewport(const VkExtent2D& extent)
{
	VkViewport viewport = {};

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	return viewport;
}

static VkRect2D scissor(const VkExtent2D& extent)
{
	VkRect2D scissor = {};

	scissor.extent = extent;
	scissor.offset = { 0, 0 };

	return scissor;
}