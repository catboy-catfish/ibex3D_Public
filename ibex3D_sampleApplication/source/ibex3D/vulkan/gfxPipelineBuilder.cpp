#include <ibex3D/vulkan/gfxPipelineBuilder.h>

#include <ibex3D/core/logger.h>

#include <vulkan/vk_enum_string_helper.h>

void i3D_vkGfxPipelineBuilder::clearEverything()
{
	dynamicStateCI = {};
	dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	
	vertexInputStateCI = {};
	vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	inputAssemblyStateCI = {};
	inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

	viewportStateCI = {};
	viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

	rasterStateCI = {};
	rasterStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

	multisampleStateCI = {};
	multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

	colorBlendAttachment = {};

	colorBlendStateCI = {};
	colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

	depthStencilStateCI = {};
	depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
}

void i3D_vkGfxPipelineBuilder::initDynamicState(const uint32_t& dynStateCount, VkDynamicState* pDynStates)
{
	dynamicStateCI = {};
	dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCI.dynamicStateCount = dynStateCount;
	dynamicStateCI.pDynamicStates = pDynStates;
}

void i3D_vkGfxPipelineBuilder::initVertexInputState(const uint32_t& bindDescCount, VkVertexInputBindingDescription* pBindDescs, const uint32_t& attribDescCount, VkVertexInputAttributeDescription* pAttribDescs)
{
	vertexInputStateCI = {};
	vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCI.vertexBindingDescriptionCount = bindDescCount;
	vertexInputStateCI.pVertexBindingDescriptions = pBindDescs;
	vertexInputStateCI.vertexAttributeDescriptionCount = attribDescCount;
	vertexInputStateCI.pVertexAttributeDescriptions = pAttribDescs;
}

void i3D_vkGfxPipelineBuilder::initInputAssemblyState(const VkPrimitiveTopology& primitiveTopology)
{
	inputAssemblyStateCI = {};
	inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCI.topology = primitiveTopology;
	inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;
}

void i3D_vkGfxPipelineBuilder::initViewportState(const VkExtent2D& extent)
{
	viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;

	viewportStateCI = {};
	viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCI.viewportCount = 1;
	viewportStateCI.pViewports = &viewport;
	viewportStateCI.scissorCount = 1;
	viewportStateCI.pScissors = &scissor;
}

void i3D_vkGfxPipelineBuilder::initRasterState(const VkPolygonMode& polygonMode, const VkCullModeFlags& cullMode, const VkFrontFace& frontFace)
{
	rasterStateCI = {};
	rasterStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;	// The length of this thing holy fuck
	rasterStateCI.depthClampEnable = VK_FALSE;
	rasterStateCI.rasterizerDiscardEnable = false;
	rasterStateCI.lineWidth = 1.0f;
	rasterStateCI.polygonMode = polygonMode;
	rasterStateCI.cullMode = cullMode;
	rasterStateCI.frontFace = frontFace;
	rasterStateCI.depthBiasEnable = VK_FALSE;
	rasterStateCI.depthBiasConstantFactor = 0.0f;
	rasterStateCI.depthBiasClamp = 0.0f;
	rasterStateCI.depthBiasSlopeFactor = 0.0f;
}

void i3D_vkGfxPipelineBuilder::initMultisampleState_enabled(const VkSampleCountFlagBits& sampleCount, const VkBool32& useSampleShading, const VkBool32& useAtoC, const VkBool32& useAtoOne)
{
	multisampleStateCI = {};
	multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCI.rasterizationSamples = sampleCount;
	multisampleStateCI.sampleShadingEnable = useSampleShading;
	multisampleStateCI.minSampleShading = 0.2f;
	multisampleStateCI.pSampleMask = nullptr;
	multisampleStateCI.alphaToCoverageEnable = useAtoC;
	multisampleStateCI.alphaToOneEnable = useAtoOne;
}

void i3D_vkGfxPipelineBuilder::initMultisampleState_disabled()
{
	multisampleStateCI = {};
	multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
}

void i3D_vkGfxPipelineBuilder::initColorBlendState_disabled()
{
	colorBlendAttachment = {};
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	colorBlendStateCI = {};
	colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCI.logicOpEnable = VK_FALSE;
	colorBlendStateCI.logicOp = VK_LOGIC_OP_COPY;
	colorBlendStateCI.attachmentCount = 1;
	colorBlendStateCI.pAttachments = &colorBlendAttachment;
	colorBlendStateCI.blendConstants[0] = 0.0f;
	colorBlendStateCI.blendConstants[1] = 0.0f;
	colorBlendStateCI.blendConstants[2] = 0.0f;
	colorBlendStateCI.blendConstants[3] = 0.0f;
}

void i3D_vkGfxPipelineBuilder::initDepthStencilState_enabled(const VkBool32& useDepthTest, const VkCompareOp& compareOp)
{
	depthStencilStateCI = {};
	depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateCI.depthTestEnable = VK_TRUE;
	depthStencilStateCI.depthWriteEnable = VK_TRUE;
	depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilStateCI.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateCI.minDepthBounds = 0.0f;
	depthStencilStateCI.maxDepthBounds = 1.0f;
	depthStencilStateCI.stencilTestEnable = VK_FALSE;
	depthStencilStateCI.front = {};
	depthStencilStateCI.back = {};
}

VkPipelineLayout i3D_vkGfxPipelineBuilder::buildPipelineLayout(VkDevice device, const uint32_t& setLayoutCount, VkDescriptorSetLayout* pSetLayouts, const uint32_t& pushConstantRangeCount, VkPushConstantRange* pPushConstantRanges)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = setLayoutCount;
	pipelineLayoutInfo.pSetLayouts = pSetLayouts;
	pipelineLayoutInfo.pushConstantRangeCount = pushConstantRangeCount;
	pipelineLayoutInfo.pPushConstantRanges = pPushConstantRanges;

	VkPipelineLayout layout = nullptr;
	
	VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the pipeline layout. VkResult: %s\n", string_VkResult(result));
		return nullptr;
	}

	return layout;
}

VkPipeline i3D_vkGfxPipelineBuilder::buildGraphicsPipeline(VkDevice device, const uint32_t& stageCount, VkPipelineShaderStageCreateInfo* pStages, VkPipelineLayout layout, VkRenderPass renderPass)
{
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = stageCount;
	pipelineInfo.pStages = pStages;
	pipelineInfo.pVertexInputState = &vertexInputStateCI;
	pipelineInfo.pInputAssemblyState = &inputAssemblyStateCI;
	pipelineInfo.pViewportState = &viewportStateCI;
	pipelineInfo.pRasterizationState = &rasterStateCI;
	pipelineInfo.pMultisampleState = &multisampleStateCI;
	pipelineInfo.pDepthStencilState = &depthStencilStateCI;
	pipelineInfo.pColorBlendState = &colorBlendStateCI;
	pipelineInfo.pDynamicState = &dynamicStateCI;
	pipelineInfo.layout = layout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	VkPipeline pipeline = nullptr;

	VkResult result = vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo, nullptr, &pipeline);

	if (result != VK_SUCCESS)
	{
		i3D_logErrorMessage("VULKAN ERROR: Failed to create the graphics pipeline. VkResult: %s\n", string_VkResult(result));
		return nullptr;
	}

	return pipeline;
}
