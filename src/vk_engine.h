// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>
#include <vector>
#include <string>
#include <functional>
#include <deque>
#include <glm/glm.hpp>
#include <unordered_map>

class PipelineBuilder {
public:
	std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
	VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo _inputAssmbly;
	VkViewport _viewport;
	VkRect2D _scissor;
	VkPipelineRasterizationStateCreateInfo _rasterier;
	VkPipelineColorBlendAttachmentState _colorBlendAttachment;
	VkPipelineDepthStencilStateCreateInfo _depthStencil;
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

struct Material {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct RenderObject
{
	Mesh* mesh;
	Material* material;
	glm::mat4 transformMatrix;
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
	
	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageViews;
	VkFormat _swapchainImageFormat;

	VkImageView _depthImageView;
	AllocatedImage _depthImage;
	VkFormat _depthFormat;

	VkCommandPool _commandPool;
	VkCommandBuffer _commandBuffer;
	VkQueue _graphicsQueue;
	uint32_t _graphicsQueueFamily;

	VkRenderPass _renderPass;
	std::vector<VkFramebuffer> _framebuffers;

	VkSemaphore _presentSem, _renderSem;
	VkFence _renderFence;

	DeletionQueue _mainDeletionQueue;

	VmaAllocator _allocator;

	std::vector<uint16_t> shader_index;
	std::vector<std::string> obj_name;
	std::vector<std::string> shader_name;

	VkPipelineLayout _meshPipelineLayout;

	// define you need pipeline
	std::vector<VkPipeline> _trianglePipelines;
	//Mesh _triangleMesh;

	std::vector<RenderObject> _renderObject;
	std::unordered_map<std::string, Mesh> _meshSet;
	std::unordered_map<std::string, Material> _material;

	//initializes everything in the engine
	void init();

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();

	// Mesh Part
	Material* create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);
	Material* get_material(const std::string& name);
	Mesh* getMesh(const std::string& name);
	void draw_object(VkCommandBuffer cmd, RenderObject* first, int count);
	
	// Math
	glm::mat4 UpdateDate(int obj_indx);

private:
	void init_vulkan();
	void init_swapchain();
	void init_commands();
	void init_default_renderpass();
	void init_framebuffers();
	void init_sync_struct();
	bool load_shader_module(const char* filePath, VkShaderModule* outShaderModule);
	void init_pipelines();
	void init_depth_image();
	void init_scene();
	bool shader_perpare(PipelineBuilder* pipelineBuilder);

	void load_mesh();
	void upload_mesh(Mesh& mesh);
};