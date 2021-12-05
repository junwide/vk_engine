
#include "vk_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vk_types.h>
#include <vk_initializers.h>
#include <VkBootstrap.h>
#include <iostream>
#include <fstream>

using namespace std;
#define VK_CHECK(x)												\
	do															\
	{															\
		VkResult err = x;										\
		if (err)												\
		{														\
			cout << "Detect Vulkan Error: " << err << endl;		\
			abort();											\
		}														\
	}while(0)

void VulkanEngine::init_vulkan()
{
	vkb::InstanceBuilder builder;
	auto inst_ret = builder.set_app_name("GTX Team Vulkan Lesson")
		.request_validation_layers(true)
		.require_api_version(1, 2, 0)
		.use_default_debug_messenger()
		.build();
	
	vkb::Instance vkb_inst = inst_ret.value();
	_instances = vkb_inst.instance;
	_debug_Message = vkb_inst.debug_messenger;

	SDL_Vulkan_CreateSurface(_window, _instances, &_surface);
	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	vkb::PhysicalDevice vkb_physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(_surface)
		.select()
		.value();

	vkb::DeviceBuilder deviceBuilder{ vkb_physicalDevice };
	vkb::Device vkb_device = deviceBuilder.build().value();
	_device = vkb_device.device;
	_choseGPU = vkb_physicalDevice.physical_device;

	_graphicsQueue = vkb_device.get_queue(vkb::QueueType::graphics).value();
	_graphicsQueueFamily = vkb_device.get_queue_index(vkb::QueueType::graphics).value(); 
}

void VulkanEngine::init_swapchain()
{
	vkb::SwapchainBuilder swapchainBuiler{ _choseGPU, _device, _surface };

	vkb::Swapchain vkb_swapchain = swapchainBuiler
		.use_default_format_selection()
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(_windowExtent.width, _windowExtent.height)
		.build()
		.value();

	_swapchain = vkb_swapchain.swapchain;
	_swapchainImages = vkb_swapchain.get_images().value();
	_swapchainImageViews = vkb_swapchain.get_image_views().value();

	_swapchainImageFormat = vkb_swapchain.image_format;

	_mainDeletionQueue.push_function([=]() {
		vkDestroySwapchainKHR(_device, _swapchain, nullptr);
		});
}

void VulkanEngine::init_commands()
{
	VkCommandPoolCreateInfo commandPoolInfo = 
		vkinit::command_pool_create_info(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_commandPool));

	VkCommandBufferAllocateInfo cmbAllocateInfo = 
		vkinit::command_buffer_allocate_info(_commandPool, 1);
	VK_CHECK(vkAllocateCommandBuffers(_device, &cmbAllocateInfo, &_commandBuffer));

	_mainDeletionQueue.push_function([=]() {
		vkDestroyCommandPool(_device, _commandPool, nullptr);
	});
}

void VulkanEngine::init_default_renderpass()
{
	VkAttachmentDescription color_attathment = {};
	color_attathment.format = _swapchainImageFormat;
	color_attathment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attathment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attathment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attathment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attathment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attathment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attathment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attathment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	VK_CHECK(vkCreateRenderPass(_device, &render_pass_info, nullptr, &_renderPass));

	_mainDeletionQueue.push_function([=]() {
		vkDestroyRenderPass(_device, _renderPass, nullptr);
		});
}

void VulkanEngine::init_framebuffers()
{
	VkFramebufferCreateInfo framebuffer_info = {};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.pNext = nullptr;
	framebuffer_info.renderPass = _renderPass;
	framebuffer_info.attachmentCount = 1;
	framebuffer_info.height = _windowExtent.height;
	framebuffer_info.width = _windowExtent.width;
	framebuffer_info.layers = 1;
	
	const uint32_t swapchain_count = static_cast<uint32_t>(_swapchainImages.size());
	_framebuffers = vector<VkFramebuffer>(swapchain_count);
	for (uint32_t i = 0; i < swapchain_count; i++)
	{
		framebuffer_info.pAttachments = &_swapchainImageViews[i];
		VK_CHECK(vkCreateFramebuffer(_device, &framebuffer_info, nullptr, &_framebuffers[i]));
		_mainDeletionQueue.push_function([=]() {
			vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
			vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
			});
	}
}

void VulkanEngine::init_sync_struct()
{
	VkFenceCreateInfo fence_info = {};
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
	VK_CHECK(vkCreateFence(_device, &fence_info, nullptr, &_renderFence));

	_mainDeletionQueue.push_function([=]() {
		vkDestroyFence(_device, _renderFence, nullptr);
	});

	VkSemaphoreCreateInfo sem_info = {};
	sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	sem_info.flags = 0;
	sem_info.pNext = nullptr;
	VK_CHECK(vkCreateSemaphore(_device, &sem_info, nullptr, &_presentSem));
	VK_CHECK(vkCreateSemaphore(_device, &sem_info, nullptr, &_renderSem));

	_mainDeletionQueue.push_function([=]() {
		vkDestroySemaphore(_device, _presentSem, nullptr);
		vkDestroySemaphore(_device, _renderSem, nullptr);
		});
}

bool VulkanEngine::load_shader_module(const char* filePath, VkShaderModule* outShaderModule)
{
	ifstream file(filePath, ios::ate | ios::binary);
	
	if (!file.is_open())
	{
		return false;
	}
	uint32_t fileSize = static_cast<uint32_t>(file.tellg());

	vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
	file.seekg(0);
	file.read((char*)buffer.data(), fileSize);
	file.close();

	VkShaderModuleCreateInfo shader_info = {};
	shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_info.pNext = nullptr;
	shader_info.codeSize = buffer.size() * sizeof(uint32_t);
	shader_info.pCode = buffer.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(_device, &shader_info, nullptr, &shaderModule) != VK_SUCCESS) {
		return false;
	}
	*outShaderModule = shaderModule;
	return true;
}

bool VulkanEngine::shader_perpare(PipelineBuilder* pipelineBuilder)
{
	
	uint8_t shader_count = static_cast<uint8_t> (shader_name.size());
	vector< VkShaderModule> targetShader(shader_count);
	for (uint16_t i = 0; i < shader_count; i++)
	{
		int flag_build = i % 2;
		string pathfile = "../../shaders/";
		pathfile = pathfile + shader_name[i];
		
		if (!load_shader_module(pathfile.c_str(), &targetShader[i]))
		{
			cout << "Error when building the shader module" << endl;
		}
		else {
			cout << "Triangle fragment shader successfully loaded" << endl;
		}
		VkShaderStageFlagBits flag = VK_SHADER_STAGE_VERTEX_BIT;
		if (flag_build){
			flag = VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		pipelineBuilder->_shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(flag, targetShader[i]));

		if (flag_build){
			_trianglePipelines.push_back(pipelineBuilder->build_pipline(_device, _renderPass));
			pipelineBuilder->_shaderStages.clear();
			_mainDeletionQueue.push_function([=]() {
				vkDestroyPipeline(_device, _trianglePipelines[i/2], nullptr);
				});
			vkDestroyShaderModule(_device, targetShader[i], nullptr);
			vkDestroyShaderModule(_device, targetShader[i-1], nullptr);
		}
		
	}
	return VK_TRUE;
}

void VulkanEngine::init_pipelines()
{
	VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();
	VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &_trianglePipelineLayout));

	PipelineBuilder pipelineBuilder;
	pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();
	pipelineBuilder._inputAssmbly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	pipelineBuilder._viewport.height = (float)_windowExtent.height;
	pipelineBuilder._viewport.width = (float)_windowExtent.width;
	pipelineBuilder._viewport.x = 0.0f;
	pipelineBuilder._viewport.y = 0.0f;
	pipelineBuilder._viewport.minDepth = 0.0f;
	pipelineBuilder._viewport.maxDepth = 1.0f;

	pipelineBuilder._scissor.offset = { 0, 0 };
	pipelineBuilder._scissor.extent = _windowExtent;

	pipelineBuilder._rasterier = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
	pipelineBuilder._mulsampling = vkinit::multisampling_state_create_info();
	pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();

	pipelineBuilder._pipelineLayout = _trianglePipelineLayout;

	// load all shader module
	shader_perpare(&pipelineBuilder);
	_mainDeletionQueue.push_function([=]() {
		vkDestroyPipelineLayout(_device, _trianglePipelineLayout, nullptr);
		});

}

VkPipeline PipelineBuilder::build_pipline(VkDevice device, VkRenderPass renderpass)
{
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;
	
	viewportState.viewportCount = 1;
	viewportState.pViewports = &_viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &_scissor;

	VkPipelineColorBlendStateCreateInfo colorblend_info = {};
	colorblend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorblend_info.pNext = nullptr;

	colorblend_info.logicOpEnable = VK_FALSE;
	colorblend_info.logicOp = VK_LOGIC_OP_COPY;
	colorblend_info.attachmentCount = 1;
	colorblend_info.pAttachments = &_colorBlendAttachment;

	// now build pipeline
	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = nullptr;

	pipeline_info.stageCount = _shaderStages.size();
	pipeline_info.pStages = _shaderStages.data();
	pipeline_info.pVertexInputState = &_vertexInputInfo;
	pipeline_info.pMultisampleState = &_mulsampling;
	pipeline_info.pInputAssemblyState = &_inputAssmbly;
	pipeline_info.pViewportState = &viewportState;
	pipeline_info.pRasterizationState = &_rasterier;
	pipeline_info.pColorBlendState = &colorblend_info;
	pipeline_info.layout = _pipelineLayout;
	pipeline_info.renderPass = renderpass;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline newPipeline;
	if (vkCreateGraphicsPipelines(
		device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &newPipeline) != VK_SUCCESS) {
		cout << "failed to create pipeline\n";
		return VK_NULL_HANDLE; // failed to create graphics pipeline
	}
	else
	{
		return newPipeline;
	}


}

void VulkanEngine::init()
{
	// We initialize SDL and create a window with it. 
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

	_window = SDL_CreateWindow(
		"Vulkan Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		_windowExtent.width,
		_windowExtent.height,
		window_flags
	);

	init_vulkan();
	
	init_swapchain();

	init_commands();

	init_default_renderpass();

	init_framebuffers();

	init_sync_struct();

	init_pipelines();
	//everything went fine
	_isInitialized = true;
}
void VulkanEngine::cleanup()
{
	if (_isInitialized) {

		vkWaitForFences(_device, 1, &_renderFence, true, 1000000000);

		_mainDeletionQueue.flush();

		vkDestroySurfaceKHR(_instances, _surface, nullptr);
		
		vkDestroyDevice(_device, nullptr);
		
		vkb::destroy_debug_utils_messenger(_instances, _debug_Message);
		vkDestroyInstance(_instances, nullptr);
		
		SDL_DestroyWindow(_window);
	}
}

void VulkanEngine::draw()
{
	VK_CHECK(vkWaitForFences(_device, 1, &_renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(_device, 1, &_renderFence));

	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, _presentSem, nullptr, &swapchainImageIndex));
	VK_CHECK(vkResetCommandBuffer(_commandBuffer, 0));

	VkCommandBuffer cmd = _commandBuffer;
	VkCommandBufferBeginInfo cmd_info = {};
	cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_info.pNext = nullptr;
	cmd_info.pInheritanceInfo = nullptr;
	cmd_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_info));  // buid the cmd buffer 

	VkClearValue clearValue;
	float flash = abs(sin(_frameNumber / 120.f));
	clearValue.color = { 0.0f, 0.0f, flash,1.0f };
	VkRenderPassBeginInfo rendpass_info = {};
	rendpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rendpass_info.pNext = nullptr;

	rendpass_info.renderPass = _renderPass;
	rendpass_info.renderArea.extent = _windowExtent;
	rendpass_info.renderArea.offset.y = 0;
	rendpass_info.renderArea.offset.x = 0;
	rendpass_info.framebuffer = _framebuffers[swapchainImageIndex];

	rendpass_info.clearValueCount = 1;
	rendpass_info.pClearValues = &clearValue;
	
	vkCmdBeginRenderPass(cmd, &rendpass_info, VK_SUBPASS_CONTENTS_INLINE);
	
	// the place
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipelines[_selectedShader]);
	vkCmdDraw(cmd, 3, 1, 0, 0);
	//

	vkCmdEndRenderPass(cmd);
	VK_CHECK(vkEndCommandBuffer(cmd));

	// submit
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = nullptr;

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit_info.pWaitDstStageMask = &waitStage;

	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &_presentSem;

	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &_renderSem;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd;

	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit_info, _renderFence)); //send to GPU
	
	//Display
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

	presentInfo.pSwapchains = &_swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &_renderSem;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));
	_frameNumber++;

}

void VulkanEngine::run()
{
	SDL_Event e;
	bool bQuit = false;

	//main loop
	while (!bQuit)
	{
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//close the window when user alt-f4s or clicks the X button			
			if (e.type == SDL_QUIT)
			{
				bQuit = true;
			}
			else if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_SPACE)
				{
					_selectedShader += 1;
					if (_selectedShader > 1)
					{
						_selectedShader = 0;
					}
				}
			}
		}

		draw();
	}
}

