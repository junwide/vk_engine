// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>
#include "vk_engine.h"
#include <fstream>
#include <string>
#include <document.h>
namespace vkinit {

	VkCommandPoolCreateInfo command_pool_create_info(
		uint32_t queueFamilyIndex, 
		VkCommandPoolCreateFlags flags = 0
		);

	VkCommandBufferAllocateInfo command_buffer_allocate_info(
		VkCommandPool pool, 
		uint32_t count = 1, 
		VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY
		);

	VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);

	VkFramebufferCreateInfo framebuffer_create_info(
		VkRenderPass renderPass, 
		VkExtent2D extent, 
		uint32_t attachment_count = 2
		);

	VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);

	VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);

	VkSubmitInfo submit_info(
		VkCommandBuffer* cmd,
		VkPipelineStageFlags* waitFlag,
		const VkSemaphore* waitSemphore,
		uint32_t wait_count,
		const VkSemaphore* signalSemphore,
		uint32_t sgnal_count
		);

	VkPresentInfoKHR present_info(
		const VkSwapchainKHR* pSwapchains,
		uint32_t   swapchainCount,
		const VkSemaphore* waitSemaphores,
		uint32_t   wait_count,
		const uint32_t* pImageIndices
	);

	VkPipelineShaderStageCreateInfo  pipeline_shader_stage_create_info(
		VkShaderStageFlagBits stage,
		VkShaderModule shaderModule
		);

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info();

	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info(VkPrimitiveTopology topology);

	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkPolygonMode polygonMode);

	VkPipelineMultisampleStateCreateInfo multisampling_state_create_info();

	VkPipelineColorBlendAttachmentState color_blend_attachment_state();

	VkPipelineLayoutCreateInfo pipeline_layout_create_info();

	VkImageCreateInfo image_create_info(
		VkFormat format, 
		VkImageUsageFlags usageFlags, 
		VkExtent3D extent
		);

	VkImageViewCreateInfo imageview_create_info(
		VkFormat format, 
		VkImage image, 
		VkImageAspectFlags aspectFlags
		);

	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info(
		bool bDepthTest,
		bool bDepthWrite,
		VkCompareOp compareOp
		);

	VkRenderPassBeginInfo renderpass_begin_info(
		VkRenderPass renderPass,
		VkExtent2D windowExtent,
		VkFramebuffer framebuffer,
		const VkClearValue* clearvalue,
		uint32_t value_count = 0,
		int32_t offset_x = 0,
		int32_t offset_y = 0
		);

	VkBufferCreateInfo buffer_create_info(
		VkStructureType s_type,
		VkBufferUsageFlags usage,
		VkDeviceSize size
	);

	VkResult pipeline_get_viewport(
		PipelineBuilder* pipeline_builder,
		VkExtent2D window,
		float maxDepth = 1.0f,
		float minDepth = 0.0,
		float offset_x = 0.0,
		float offset_y = 0.0
	);

	VkResult pipeline_get_scissor(
		PipelineBuilder* pipeline_builder,
		VkRect2D window
	);

	VkResult pipelineLayoutinfo_get_pushConstant(
		std::vector<VkPushConstantRange>& push_constant,
		VkShaderStageFlags flag,
		uint32_t struct_size,
		uint32_t pushconstant_count = 1
	);

	VkResult shadername_get(
		std::vector<std::string>& shader_name,
		std::vector<uint16_t> &shader_index,
		std::vector<std::string> &obj_name,
		rapidjson::Document &object
	);

	VkDescriptorSetLayoutBinding descriptor_setlayout_binding(
		uint32_t              binding,
		VkDescriptorType      descriptorType,
		uint32_t              descriptorCount,
		VkShaderStageFlags    stageFlags
	);

	VkDescriptorSetLayoutCreateInfo descriptor_setlayout_info(
		uint32_t   bindingCount,
		VkDescriptorSetLayoutBinding& pBindings,
		VkDescriptorSetLayoutCreateFlags  flags = 0
	);

	VkDescriptorPoolCreateInfo descriptorpool_create_info(
		uint32_t  maxSets,
		VkDescriptorPoolSize* pPoolSizes,
		uint32_t poolSizeCount,
		VkDescriptorPoolCreateFlags    flags = 0
	);

	VkDescriptorSetAllocateInfo descriptorset_allocate_info(
		VkDescriptorPool     descriptorPool,
		uint32_t             descriptorSetCount,
		VkDescriptorSetLayout& pSetLayouts
	);

	VkDescriptorBufferInfo descriptor_buffer_info(
		VkBuffer        buffer,
		VkDeviceSize    offset,
		VkDeviceSize    range
	);

	VkWriteDescriptorSet write_descriptor_buffer(
		VkDescriptorType type,
		VkDescriptorSet dstSet,
		VkDescriptorBufferInfo* bufferInfo,
		uint32_t binding
	);
}

namespace file_box {
	VkResult readfile(
					rapidjson::Document& object,
					const std::string file_name, 
					const std::string file_path = "../../json/");


}

