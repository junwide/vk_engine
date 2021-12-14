
#include "vk_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vk_types.h>
#include <vk_initializers.h>
#include <VkBootstrap.h>
#include <iostream>
#include <fstream>
#include <glm/gtx/transform.hpp>
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
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

	VmaAllocatorCreateInfo allocator_info = {};
	allocator_info.instance = _instances;
	allocator_info.device = _device;
	allocator_info.physicalDevice = _choseGPU;
	vmaCreateAllocator(&allocator_info, &_allocator);
	_mainDeletionQueue.push_function([=]() {
		vmaDestroyAllocator(_allocator);
		});

	vkGetPhysicalDeviceProperties(_choseGPU, &_gpuProperties);
	cout << "The GPU has a minimum buffer alignment of " << _gpuProperties.limits.minUniformBufferOffsetAlignment << endl;
	// Test;
	rapidjson::Document object_json;
	VK_CHECK(file_box::readfile(object_json, "shader_config.json"));
	vkinit::shadername_get(shader_name, shader_index, obj_name, object_json);
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
	init_depth_image();
}

void VulkanEngine::init_commands()
{
	VkCommandPoolCreateInfo commandPoolInfo = 
		vkinit::command_pool_create_info(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));

		VkCommandBufferAllocateInfo cmbAllocateInfo =
			vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(_device, &cmbAllocateInfo, &_frames[i]._commandBuffer));

		_mainDeletionQueue.push_function([=]() {
			vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);
			});
	}
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

	VkAttachmentDescription depth_attachment = {};
	depth_attachment.flags = 0;
	depth_attachment.format = _depthFormat;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	VkAttachmentDescription attachments[2] = { color_attathment,depth_attachment };

	render_pass_info.attachmentCount = 2;
	render_pass_info.pAttachments = &attachments[0];
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	VK_CHECK(vkCreateRenderPass(_device, &render_pass_info, nullptr, &_renderPass));

	_mainDeletionQueue.push_function([=]() {
		vkDestroyRenderPass(_device, _renderPass, nullptr);
		});
}

void VulkanEngine::init_framebuffers()
{
	VkFramebufferCreateInfo framebuffer_info = vkinit::framebuffer_create_info(
												_renderPass,
												_windowExtent);
	
	const uint32_t swapchain_count = static_cast<uint32_t>(_swapchainImages.size());
	_framebuffers = vector<VkFramebuffer>(swapchain_count);
	for (uint32_t i = 0; i < swapchain_count; i++)
	{
		VkImageView attachments[2];
		attachments[0] = _swapchainImageViews[i];
		attachments[1] = _depthImageView;
		framebuffer_info.pAttachments = attachments;
		VK_CHECK(vkCreateFramebuffer(_device, &framebuffer_info, nullptr, &_framebuffers[i]));
		
		_mainDeletionQueue.push_function([=]() {
			vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
			vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
			});
	}
}

void VulkanEngine::init_sync_struct()
{
	//create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkFenceCreateInfo fence_info = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo sem_info = vkinit::semaphore_create_info();
	
	

	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		VK_CHECK(vkCreateFence(_device, &fence_info, nullptr, &_frames[i]._renderFence));

		_mainDeletionQueue.push_function([=]() {
			vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
			});


		VK_CHECK(vkCreateSemaphore(_device, &sem_info, nullptr, &_frames[i]._presentSem));
		VK_CHECK(vkCreateSemaphore(_device, &sem_info, nullptr, &_frames[i]._renderSem));

		_mainDeletionQueue.push_function([=]() {
			vkDestroySemaphore(_device, _frames[i]._presentSem, nullptr);
			vkDestroySemaphore(_device, _frames[i]._renderSem, nullptr);
			});
	}
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
	VkPipeline _trianglePipelines;
	uint8_t shader_count = static_cast<uint8_t> (shader_index.size());
	vector<VkShaderModule> targetShader(shader_count);
	VertexInputDescription vertexDescription = Vertex::get_vertex_description();
	for (uint16_t i = 0, j = 0; i < shader_count; i++)
	{
		int flag_build = i % 2;
		string pathfile = "../../shaders/";
		pathfile = pathfile + shader_name[shader_index[i]];
		
		pipelineBuilder->_vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
		pipelineBuilder->_vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();
		pipelineBuilder->_vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
		pipelineBuilder->_vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

		pipelineBuilder->_pipelineLayout = _meshPipelineLayout;
		
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
			pipelineBuilder->_depthStencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
			_trianglePipelines = pipelineBuilder->build_pipline(_device, _renderPass);
			create_material(_trianglePipelines, _meshPipelineLayout, "defaultmesh");
			pipelineBuilder->_shaderStages.clear();

			_mainDeletionQueue.push_function([=]() {
				vkDestroyPipeline(_device, _trianglePipelines, nullptr);
				});

			vkDestroyShaderModule(_device, targetShader[i], nullptr);
			vkDestroyShaderModule(_device, targetShader[i-1], nullptr);
		}
		
	}
	return VK_TRUE;
}

void VulkanEngine::init_pipelines()
{
	VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = vkinit::pipeline_layout_create_info();

	vector<VkPushConstantRange> push_constant;
	VK_CHECK(vkinit::pipelineLayoutinfo_get_pushConstant(
														push_constant,
														VK_SHADER_STAGE_VERTEX_BIT,
														sizeof(MeshPushConstants),
														1));
	mesh_pipeline_layout_info.pushConstantRangeCount = 1;
	mesh_pipeline_layout_info.pPushConstantRanges = &push_constant[0];
	mesh_pipeline_layout_info.pSetLayouts = &_globalSetLayout;
	mesh_pipeline_layout_info.setLayoutCount = 1;
	VK_CHECK(vkCreatePipelineLayout(_device, &mesh_pipeline_layout_info, nullptr, &_meshPipelineLayout));

	PipelineBuilder pipelineBuilder;
	pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();
	pipelineBuilder._inputAssmbly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	
	VK_CHECK(vkinit::pipeline_get_viewport(&pipelineBuilder, _windowExtent););

	VkRect2D scissor_set;
	scissor_set.offset = { 0, 0 };
	scissor_set.extent = _windowExtent;
	VK_CHECK(vkinit::pipeline_get_scissor(&pipelineBuilder, scissor_set));

	pipelineBuilder._rasterier = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
	pipelineBuilder._mulsampling = vkinit::multisampling_state_create_info();
	pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();

	// load all shader module
	shader_perpare(&pipelineBuilder);
	
	_mainDeletionQueue.push_function([=]() {
		vkDestroyPipelineLayout(_device, _meshPipelineLayout, nullptr);
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
	pipeline_info.pDepthStencilState = &_depthStencil;
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

void VulkanEngine::init_depth_image()
{
	VkExtent3D depthImageExtent = {
		_windowExtent.width,
		_windowExtent.height,
		1
	};
	_depthFormat = VK_FORMAT_D32_SFLOAT;
	VkImageCreateInfo depth_info = vkinit::image_create_info(_depthFormat, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);
	VmaAllocationCreateInfo depth_allocat_info = {};
	depth_allocat_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	depth_allocat_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(_allocator, &depth_info ,&depth_allocat_info, 
					&_depthImage._image, &_depthImage._allocation,
					nullptr);
	VkImageViewCreateInfo depthview_info = vkinit::imageview_create_info(_depthFormat,
		_depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);

	VK_CHECK(vkCreateImageView(_device, &depthview_info, nullptr, &_depthImageView));
	_mainDeletionQueue.push_function([=]() {
		vkDestroyImageView(_device, _depthImageView, nullptr);
		vmaDestroyImage(_allocator, _depthImage._image, _depthImage._allocation);
		});
}

void VulkanEngine::upload_mesh(Mesh& mesh)
{
	VkBufferCreateInfo buffer_info = vkinit::buffer_create_info(
										VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
										VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
										mesh._vertices.size() * sizeof(Vertex)
									);
	// allocation not allocator
	VmaAllocationCreateInfo vmaalloc_info = {};
	vmaalloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	
	VK_CHECK(vmaCreateBuffer(_allocator,
		&buffer_info,
		&vmaalloc_info,
		&mesh._vertexBuffer._buffer,
		&mesh._vertexBuffer._allocation,
		nullptr));

	_mainDeletionQueue.push_function([=]() {
		vmaDestroyBuffer(_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation);
		});
	
	void* data;
	vmaMapMemory(_allocator, mesh._vertexBuffer._allocation, &data);
	memcpy(data, mesh._vertices.data(), mesh._vertices.size() * sizeof(Vertex));
	vmaUnmapMemory(_allocator, mesh._vertexBuffer._allocation);
}

void VulkanEngine::load_mesh()
{
	////make the array 3 vertices long
	//_triangleMesh._vertices.resize(3);

	////vertex positions
	//_triangleMesh._vertices[0].position = { 1.f, 1.f, 0.0f };
	//_triangleMesh._vertices[1].position = { -1.f, 1.f, 0.0f };
	//_triangleMesh._vertices[2].position = { 0.f,-1.f, 0.0f };

	////vertex colors, all green
	//_triangleMesh._vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
	//_triangleMesh._vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
	//_triangleMesh._vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green

	//we don't care about the vertex normals
	string file_path = "../../assets/";
	for (string objname: obj_name)
	{
		Mesh mesh_each;
		string file = file_path + objname;
		mesh_each.load_from_obj(file.c_str());
		upload_mesh(mesh_each);
		_meshSet[objname] = mesh_each;
	}
}

FrameData& VulkanEngine::get_current_frame()
{
	return _frames[_frameNumber % FRAME_OVERLAP];
}

void VulkanEngine::UpdateDate(int obj_index)
{
	glm::vec3 camPos = _renderObject[_selectedShader].camPos + _movestatus.camPos;
	_renderObject[_selectedShader].camPos = camPos;
	_movestatus.camPos = { 0.f, 0.f, 0.f };

	glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
	//camera projection
	glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
	projection[1][1] *= -1;
	//model rotation
	_renderObject[obj_index].transformMatrix = _movestatus.transformMatrix;
	//_renderObject[_selectedShader].transformMatrix = model;
	//_movestatus.transformMatrix = glm::mat4{ 1.f };


	GPUCameraData camData;
	camData.projection = projection;
	camData.view = view;
	//camData.viewproj = projection * view;

	void* data;
	vmaMapMemory(_allocator, get_current_frame().cameraBuffer._allocation, &data);
	memcpy(data, &camData, sizeof(GPUCameraData));
	vmaUnmapMemory(_allocator, get_current_frame().cameraBuffer._allocation);

	float framed = (_frameNumber / 120.f);
	_senceParameters.ambientColor = { sin(framed),0,cos(framed),1 };

	char* sceneData;
	vmaMapMemory(_allocator, _senneParameterBuffer._allocation, (void**)&sceneData);
	int frameIndex = _frameNumber % FRAME_OVERLAP;

	sceneData += pad_uniform_buffer_size(sizeof(GPUSenceData)) * frameIndex;
	memcpy(sceneData, &_senceParameters, sizeof(GPUSenceData));
	vmaUnmapMemory(_allocator, _senneParameterBuffer._allocation);


	return;
}

Material* VulkanEngine::create_material(VkPipeline pipeline, VkPipelineLayout layout, const string& name)
{
	Material mat;
	mat.pipeline = pipeline;
	mat.pipelineLayout = layout;
	_material[name] = mat;
	return &_material[name];
}

Material* VulkanEngine::get_material(const std::string& name)
{
	auto it = _material.find(name);
	if (it == _material.end())
	{
		return nullptr;
	}
	else
	{
		return &(*it).second;
	}
}

Mesh* VulkanEngine::getMesh(const std::string& name)
{
	auto it = _meshSet.find(name);
	if (it == _meshSet.end())
	{
		return nullptr;
	}
	else
	{
		return &(*it).second;
	}
}
void VulkanEngine::draw_object(VkCommandBuffer cmd, RenderObject* first, int count)
{
	//glm::vec3 camPos = { 0.f ,-6.f, -10.f };
	//glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
	//glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
	//projection[1][1] *= -1;

	//Mesh* lastMesh = nullptr;
	//Material* lastMaterial = nullptr;
	//for (int i = 0; i < count; i++)
	//{
	//	RenderObject& obj = first[i];
	//	if (obj.material != lastMaterial)
	//	{
	//		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, obj.material->pipeline);
	//		lastMaterial = obj.material;
	//	}
	//	glm::mat4 model = obj.transformMatrix;
	//	glm::mat4 mvp_Matrix = projection* view * model;

	//	MeshPushConstants constant;
	//	constant.render_matrix = mvp_Matrix;
	//	vkCmdPushConstants(cmd, obj.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants),&constant);
	//	if (obj.mesh != lastMesh) {
	//		//bind the mesh vertex buffer with offset 0
	//		VkDeviceSize offset = 0;
	//		vkCmdBindVertexBuffers(cmd, 0, 1, &obj.mesh->_vertexBuffer._buffer, &offset);
	//		lastMesh = obj.mesh;
	//	}
	//	vkCmdDraw(cmd, obj.mesh->_vertices.size(), 1, 0, 0);
	//}
	UpdateDate(_selectedShader);
	uint32_t uniform_offset = pad_uniform_buffer_size(sizeof(GPUSenceData)) * (_frameNumber % FRAME_OVERLAP);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _renderObject[_selectedShader].material->pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _renderObject[_selectedShader].material->pipelineLayout, 0, 1, &get_current_frame().globalDescriptor, 1, &uniform_offset);
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &_renderObject[_selectedShader].mesh->_vertexBuffer._buffer, &offset);
	MeshPushConstants constants;
	constants.render_matrix = _renderObject[_selectedShader].transformMatrix;
	//upload the matrix to the GPU via push constants
	vkCmdPushConstants(cmd, _renderObject[_selectedShader].material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
	//we can now draw
	vkCmdDraw(cmd, _renderObject[_selectedShader].mesh->_vertices.size(), 1, 0, 0);
	return;
}

void VulkanEngine::init_scene()
{
	for ( auto name: obj_name)
	{
		RenderObject mesh_obj;
		mesh_obj.mesh = getMesh(name);
		mesh_obj.material = get_material("defaultmesh");
		mesh_obj.transformMatrix = glm::mat4{ 1.0f };
		mesh_obj.camPos = { 0.f,0.f ,-2.f };
		_renderObject.push_back(mesh_obj);
	}
	//_renderObject.push_back(monkey);
	//for (int x = -20; x <= 20; x++) {
	//	for (int y = -20; y <= 20; y++) {

	//		RenderObject tri;
	//		tri.mesh = getMesh("triangle");
	//		tri.material = get_material("defaultmesh");
	//		glm::mat4 translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(x, 0, y));
	//		glm::mat4 scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(0.2, 0.2, 0.2));
	//		tri.transformMatrix = translation * scale;

	//		_renderObject.push_back(tri);
	//	}
	//}
}
AllocatedBuffer VulkanEngine::create_buffer(
	size_t allocSize,
	VkBufferUsageFlags usage,
	VmaMemoryUsage memoryUsage
) 
{
	VkBufferCreateInfo buffer_info = vkinit::buffer_create_info(
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		usage,
		allocSize
	);
	// allocation not allocator
	VmaAllocationCreateInfo vmaalloc_info = {};
	vmaalloc_info.usage = memoryUsage;

	AllocatedBuffer newBuffer;

	VK_CHECK(vmaCreateBuffer(_allocator,
		&buffer_info,
		&vmaalloc_info,
		&newBuffer._buffer,
		&newBuffer._allocation,
		nullptr));

	_mainDeletionQueue.push_function([=]() {
		vmaDestroyBuffer(_allocator, newBuffer._buffer, newBuffer._allocation);
		});
	return newBuffer;
}
void VulkanEngine::init_descriptors()
{
	std::vector<VkDescriptorPoolSize> sizes =
	{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 }
	};
	//information about the binding.
	VkDescriptorSetLayoutBinding camBufferBinding = vkinit::descriptor_setlayout_binding(
				/* it's a uniform buffer binding*/		0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			/* we use it from the vertex shader*/		1, VK_SHADER_STAGE_VERTEX_BIT
														);
	VkDescriptorSetLayoutBinding senceBufferBinding = vkinit::descriptor_setlayout_binding(
		/* it's a uniform buffer binding*/		1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		/* we use it from the vertex shader*/   1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
	);
	VkDescriptorSetLayoutBinding bindings[] = { camBufferBinding,senceBufferBinding };
	VkDescriptorSetLayoutCreateInfo setInfo = vkinit::descriptor_setlayout_info(2, bindings[0]);
	setInfo.pBindings = bindings;
	VK_CHECK(vkCreateDescriptorSetLayout(_device, &setInfo, nullptr, &_globalSetLayout));
	
	_mainDeletionQueue.push_function([=]() {
		vkDestroyDescriptorSetLayout(_device, _globalSetLayout, nullptr);
		});

	VkDescriptorPoolCreateInfo pool_info = vkinit::descriptorpool_create_info(10, sizes.data(), sizes.size() );

	VK_CHECK(vkCreateDescriptorPool(_device, &pool_info, nullptr, &_descriptorPool));
	_mainDeletionQueue.push_function([=]() {
		vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
		});


	const size_t sceneParamBufferSize = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSenceData));
	_senneParameterBuffer = create_buffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		_frames[i].cameraBuffer = create_buffer(
									sizeof(GPUCameraData),
									VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
									VMA_MEMORY_USAGE_CPU_TO_GPU
									);
		VkDescriptorSetAllocateInfo allocInfo = vkinit::descriptorset_allocate_info(_descriptorPool, 1, _globalSetLayout);
		VK_CHECK(vkAllocateDescriptorSets(_device, &allocInfo, &_frames[i].globalDescriptor));

		//_mainDeletionQueue.push_function([=]() {
		//	vkFreeDescriptorSets(_device, _descriptorPool, 1, &_frames[i].globalDescriptor);
		//});

		VkDescriptorBufferInfo cam_info = vkinit::descriptor_buffer_info(
												_frames[i].cameraBuffer._buffer, 
												0, sizeof(GPUCameraData)
												);
		VkDescriptorBufferInfo sence_info = vkinit::descriptor_buffer_info(
											_senneParameterBuffer._buffer,
											0, sizeof(GPUSenceData)
										);

		VkWriteDescriptorSet cameraWrite = vkinit::write_descriptor_buffer(
											VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
											_frames[i].globalDescriptor, &cam_info, 0);
		
		VkWriteDescriptorSet senceWrite = vkinit::write_descriptor_buffer(
										VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
										_frames[i].globalDescriptor,
										&sence_info,
										1
									);
		VkWriteDescriptorSet setWrites[] = { cameraWrite,senceWrite };

		vkUpdateDescriptorSets(_device, 2, setWrites, 0, nullptr);
	}
}

void VulkanEngine::key_event_process(int32_t keycode)
{
	int mode_count = obj_name.size();
	int selected = keycode - SDLK_1;
	if (selected < 9 && selected >= 0)
	{
		_selectedShader = selected;
		return;
	}

	switch (keycode)
	{
	case SDLK_a:
		 _movestatus.transformMatrix *= glm::rotate(glm::mat4{ 1.0f }, glm::radians(5.f), glm::vec3(0, 1, 0));
		break;
	case SDLK_d:
		_movestatus.transformMatrix *= glm::rotate(glm::mat4{ 1.0f }, glm::radians(-5.f), glm::vec3(0, 1, 0));
		break;
	case SDLK_w:
		_movestatus.transformMatrix *= glm::rotate(glm::mat4{ 1.0f }, glm::radians(5.f), glm::vec3(1, 0, 0));
		break;
	case SDLK_s:
		_movestatus.transformMatrix *= glm::rotate(glm::mat4{ 1.0f }, glm::radians(-5.f), glm::vec3(1, 0, 0));
		break;
	case SDLK_q:
		_movestatus.camPos[2] = -1.f;
		break;
	case SDLK_e:
		_movestatus.camPos[2] = 1.f;
		break;
	case SDLK_x:
		_movestatus.current_handle[0] = _movestatus.current_handle[0] ^ 0x01;
		break;
	case SDLK_z:
		_movestatus.camPos[_movestatus.current_handle[0]] = -1.f;
		break;
	case SDLK_c:
		_movestatus.camPos[_movestatus.current_handle[0]] = 1.f;
		break;
	default:
		break;
	}
}

size_t VulkanEngine::pad_uniform_buffer_size(size_t originalSize)
{
	size_t minUboAlignment = _gpuProperties.limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAlignment > 0)
	{
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
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

	init_descriptors();

	init_pipelines();
	//everything went fine
	load_mesh();

	init_scene();

	_isInitialized = true;
}
void VulkanEngine::cleanup()
{
	if (_isInitialized) {
		--_frameNumber; // in draw call last it ++
		vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000);

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
	VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));
	VK_CHECK(vkResetCommandBuffer(get_current_frame()._commandBuffer, 0));

	uint32_t swapchainImageIndex; 
	VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, get_current_frame()._presentSem, nullptr, &swapchainImageIndex));
	

	VkCommandBuffer cmd = get_current_frame()._commandBuffer;
	VkCommandBufferBeginInfo cmd_info = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_info));  // buid the cmd buffer 

	VkClearValue clearValue;
	float flash = abs(sin(_frameNumber / 120.f));
	clearValue.color = { 0.0f, 0.0f, flash,1.0f };

	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	VkClearValue clearValues[2] = { clearValue, depthClear };

	VkRenderPassBeginInfo rendpass_info = vkinit::renderpass_begin_info(
										_renderPass,
										_windowExtent,
										_framebuffers[swapchainImageIndex],
										&clearValues[0], 2
										);
	
	vkCmdBeginRenderPass(cmd, &rendpass_info, VK_SUBPASS_CONTENTS_INLINE);
	
	// the place
	//if (_selectedShader == 1)
	//{
	draw_object(cmd, _renderObject.data(), _renderObject.size());
	//}
	//vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipelines[_selectedShader]);


	vkCmdEndRenderPass(cmd);
	VK_CHECK(vkEndCommandBuffer(cmd));

	// submit
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submit_info = vkinit::submit_info(
							&cmd,
							&waitStage,
							&get_current_frame()._presentSem, 1,
							&get_current_frame()._renderSem, 1
							);
	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit_info, get_current_frame()._renderFence)); //send to GPU
	
	//Display
	VkPresentInfoKHR presentInfo = vkinit::present_info(
								&_swapchain, 1,
								&get_current_frame()._renderSem, 1,
								&swapchainImageIndex
								);
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
				key_event_process(e.key.keysym.sym);
			}
		}

		draw();
	}
}

