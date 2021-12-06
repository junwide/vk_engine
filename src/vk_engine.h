// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>
#include <vector>
#include <string>
#include <functional>
#include <deque>
#include <glm/glm.hpp>

class PipelineBuilder {
public:
	std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
	VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo _inputAssmbly;
	VkViewport _viewport;
	VkRect2D _scissor;
	VkPipelineRasterizationStateCreateInfo _rasterier;
	VkPipelineColorBlendAttachmentState _colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo _mulsampling;
	VkPipelineLayout _pipelineLayout;

	VkPipeline build_pipline(VkDevice device, VkRenderPass renderpass);
};

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}
		void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); //call the function
		}
		deletors.clear();
	}
};

struct MeshPushConstants {
	glm::vec4 data;
	glm::mat4 render_matrix;
};

class VulkanEngine {
public:

	bool _isInitialized{ false };
	int _selectedShader{ 0 };
	int _frameNumber {0};

	VkExtent2D _windowExtent{ 1700 , 900 };

	struct SDL_Window* _window{ nullptr };

	VkInstance _instances;
	VkPhysicalDevice _choseGPU;
	VkDevice _device;
	VkSurfaceKHR _surface;
	VkDebugUtilsMessengerEXT _debug_Message;

	VkSwapchainKHR _swapchain;
	VkFormat _swapchainImageFormat;
	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageViews;

	VkCommandPool _commandPool;
	VkCommandBuffer _commandBuffer;
	VkQueue _graphicsQueue;
	uint32_t _graphicsQueueFamily;

	VkRenderPass _renderPass;
	std::vector<VkFramebuffer> _framebuffers;

	VkSemaphore _presentSem, _renderSem;
	VkFence _renderFence;

	VkPipelineLayout _trianglePipelineLayout;
	VkPipelineLayout _meshPipelineLayout;

	// define you need pipeline
	std::vector<VkPipeline> _trianglePipelines;

	DeletionQueue _mainDeletionQueue;

	std::vector<uint16_t> shader_index{ 0,1,2,3,4,3,4,3};
	std::vector<uint16_t> mesh_index_shader{4,6};
	std::vector<std::string> shader_name{
		{"triangle.vert.spv"},
		{"triangle.frag.spv"},
		{"colored_triangle.vert.spv"},
		{"colored_triangle.frag.spv"},
		{"tri_mesh.vert.spv"}
	};

	VmaAllocator _allocator;
	Mesh _triangleMesh;
	Mesh _monkeyMesh;
	//initializes everything in the engine
	void init();

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();
private:
	void init_vulkan();
	void init_swapchain();
	void init_commands();
	void init_default_renderpass();
	void init_framebuffers();
	void init_sync_struct();
	bool load_shader_module(const char* filePath, VkShaderModule* outShaderModule);
	void init_pipelines();
	bool shader_perpare(PipelineBuilder* pipelineBuilder);

	// Mesh Part
	void load_mesh();
	void upload_mesh(Mesh& mesh);
	glm::mat4 UpdateDate();
};