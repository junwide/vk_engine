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

	VkCommandBufferBeginInfo vkinit::command_buffer_begin_info(VkCommandBufferUsageFlags flags /*= 0*/)
	{
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;

		info.pInheritanceInfo = nullptr;
		info.flags = flags;
		return info;
	}

	VkFramebufferCreateInfo vkinit::framebuffer_create_info(
		VkRenderPass renderPass,
		VkExtent2D extent,
		uint32_t attachment_count
		)
	{
		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.pNext = nullptr;

		info.renderPass = renderPass;
		info.attachmentCount = attachment_count;
		info.width = extent.width;
		info.height = extent.height;
		info.layers = 1;

		return info;
	}

	VkFenceCreateInfo vkinit::fence_create_info(VkFenceCreateFlags flags /*= 0*/)
	{
		VkFenceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.pNext = nullptr;

		info.flags = flags;

		return info;
	}

	VkSemaphoreCreateInfo vkinit::semaphore_create_info(VkSemaphoreCreateFlags flags /*= 0*/)
	{
		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = flags;
		return info;
	}

	VkSubmitInfo vkinit::submit_info(
		VkCommandBuffer* cmd,
		VkPipelineStageFlags* waitFlag,
		const VkSemaphore* waitSemphore,
		uint32_t wait_count,
		const VkSemaphore* signalSemphore,
		uint32_t sgnal_count
	)
	{
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.pNext = nullptr;

		info.pWaitDstStageMask = waitFlag;

		info.waitSemaphoreCount = wait_count;
		info.pWaitSemaphores = waitSemphore;

		info.signalSemaphoreCount = sgnal_count;
		info.pSignalSemaphores = signalSemphore;

		info.commandBufferCount = 1;
		info.pCommandBuffers = cmd;

		return info;
	}

	VkPresentInfoKHR vkinit::present_info(
		const VkSwapchainKHR* swapchains,
		uint32_t   swapchain_Count,
		const VkSemaphore* waitSemaphores,
		uint32_t   wait_count,
		const uint32_t* pImageIndices
	)
	{
		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.pNext = nullptr;

		info.swapchainCount = swapchain_Count;
		info.pSwapchains = swapchains;
		info.pWaitSemaphores = waitSemaphores;
		info.waitSemaphoreCount = wait_count;
		info.pImageIndices = pImageIndices;

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

	VkImageCreateInfo vkinit::image_create_info(
		VkFormat format,
		VkImageUsageFlags usageFlags,
		VkExtent3D extent
	) 
	{
		VkImageCreateInfo image_info = {};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.pNext = nullptr;

		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.format = format;
		image_info.extent = extent;

		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.usage = usageFlags;

		return image_info;
	}

	VkImageViewCreateInfo vkinit::imageview_create_info(
		VkFormat format,
		VkImage image,
		VkImageAspectFlags aspectFlags
	)
	{
		VkImageViewCreateInfo imageView_info = {};
		imageView_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageView_info.pNext = nullptr;
		
		imageView_info.format = format;
		imageView_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageView_info.image = image;
		imageView_info.subresourceRange.layerCount = 1;
		imageView_info.subresourceRange.levelCount = 1;
		imageView_info.subresourceRange.baseMipLevel = 0;
		imageView_info.subresourceRange.baseArrayLayer = 0;
		imageView_info.subresourceRange.aspectMask = aspectFlags;

		return imageView_info;
	}

	VkPipelineDepthStencilStateCreateInfo vkinit::depth_stencil_create_info(
		bool bDepthTest,
		bool bDepthWrite,
		VkCompareOp compareOp
	)
	{
		VkPipelineDepthStencilStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		info.pNext = nullptr;

		info.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE;
		info.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE;
		info.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
		info.depthBoundsTestEnable = VK_FALSE;
		info.minDepthBounds = 0.0f; // Optional
		info.maxDepthBounds = 1.0f; // Optional
		info.stencilTestEnable = VK_FALSE;

		return info;
	}

	VkRenderPassBeginInfo vkinit::renderpass_begin_info(
		VkRenderPass renderPass,
		VkExtent2D windowExtent,
		VkFramebuffer framebuffer,
		const VkClearValue* clearvalue,
		uint32_t value_count,
		int32_t offset_x,
		int32_t offset_y
		)
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.pNext = nullptr;

		info.renderPass = renderPass;
		info.renderArea.offset.x = offset_x;
		info.renderArea.offset.y = offset_y;
		info.renderArea.extent = windowExtent;
		info.clearValueCount = value_count;
		info.pClearValues = clearvalue;
		info.framebuffer = framebuffer;

		return info;
	}
}

