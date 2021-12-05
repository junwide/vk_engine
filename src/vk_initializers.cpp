#include <vk_initializers.h>

namespace vkinit {
	VkCommandPoolCreateInfo vkinit::command_pool_create_info
		(uint32_t queueFamilyIndex, 
		VkCommandPoolCreateFlags flags
		)
	{
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.pNext = nullptr;

		info.queueFamilyIndex = queueFamilyIndex;
		info.flags = flags;
		return info;
	}

	VkCommandBufferAllocateInfo vkinit::command_buffer_allocate_info
		(VkCommandPool pool, 
		uint32_t count, 
		VkCommandBufferLevel level
		)
	{
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.pNext = nullptr;

		info.commandPool = pool;
		info.commandBufferCount = count;
		info.level = level;
		return info;
	}

	VkPipelineShaderStageCreateInfo  vkinit::pipeline_shader_stage_create_info(
		VkShaderStageFlagBits stage,
		VkShaderModule shaderModule
	)
	{
		VkPipelineShaderStageCreateInfo shader_info = {};
		shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_info.pNext = nullptr;
		shader_info.stage = stage;
		shader_info.module = shaderModule;
		shader_info.pName = "main";
		return shader_info;
	}

	VkPipelineVertexInputStateCreateInfo vkinit::vertex_input_state_create_info()
	{
		VkPipelineVertexInputStateCreateInfo vertex_info = {};
		vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_info.pNext = nullptr;
		vertex_info.vertexAttributeDescriptionCount = 0;
		vertex_info.vertexBindingDescriptionCount = 0;

		return vertex_info;
	}

	VkPipelineInputAssemblyStateCreateInfo vkinit::input_assembly_create_info(VkPrimitiveTopology topology)
	{
		VkPipelineInputAssemblyStateCreateInfo assembly_info = {};
		assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assembly_info.pNext = nullptr;
		assembly_info.topology = topology;
		assembly_info.primitiveRestartEnable = VK_FALSE;
		return assembly_info;
	}

	VkPipelineRasterizationStateCreateInfo vkinit::rasterization_state_create_info(VkPolygonMode polygonMode)
	{
		VkPipelineRasterizationStateCreateInfo raster_info = {};
		raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster_info.pNext = nullptr;
		raster_info.depthClampEnable = VK_FALSE;
		raster_info.rasterizerDiscardEnable = VK_FALSE;
		
		raster_info.polygonMode = polygonMode;
		raster_info.lineWidth = 1.0f
			;
		raster_info.cullMode = VK_CULL_MODE_NONE;
		raster_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		
		raster_info.depthBiasEnable = VK_FALSE;
		raster_info.depthBiasConstantFactor = 0.0f;
		raster_info.depthBiasClamp = 0.0f;
		raster_info.depthBiasSlopeFactor = 0.0f;

		return raster_info;
	}

	VkPipelineMultisampleStateCreateInfo vkinit::multisampling_state_create_info()
	{
		VkPipelineMultisampleStateCreateInfo mulsample_info = {};
		mulsample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mulsample_info.pNext = nullptr;
		mulsample_info.sampleShadingEnable = VK_FALSE;
		mulsample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		mulsample_info.minSampleShading = 1.0f;
		mulsample_info.pSampleMask = nullptr;
		mulsample_info.alphaToCoverageEnable = VK_FALSE;
		mulsample_info.alphaToOneEnable = VK_FALSE;
		return mulsample_info;
	}
	VkPipelineColorBlendAttachmentState vkinit::color_blend_attachment_state() {
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
											VK_COLOR_COMPONENT_G_BIT |
											VK_COLOR_COMPONENT_B_BIT | 
											VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		return colorBlendAttachment;
	}

	VkPipelineLayoutCreateInfo vkinit::pipeline_layout_create_info() {
		VkPipelineLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layout_info.pNext = nullptr;

		//empty defaults
		layout_info.flags = 0;
		layout_info.setLayoutCount = 0;
		layout_info.pSetLayouts = nullptr;
		layout_info.pushConstantRangeCount = 0;
		layout_info.pPushConstantRanges = nullptr;
		return layout_info;
	}
}

