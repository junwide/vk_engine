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
constexpr unsigned int FRAME_OVERLAP = 2;

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
	VkDescriptorSet textureSet{ VK_NULL_HANDLE };
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct RenderObject
{
	Mesh* mesh;
	Material* material;
	glm::vec3 camPos;
	glm::mat4 transformMatrix;
};

struct movestatus
{
	uint8_t current_handle[2] = { 0, 0 };
	glm::vec3 camPos = { 0.f, 0.f ,0.f };
	glm::mat4 transformMatrix = glm::mat4{ 1.0f };
};

struct GPUCameraData {
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 viewproj;
};

struct GPUObjectData {
	glm::mat4 modelMatrix;
};

struct GPUSenceData{
	glm::vec4 fogColor;
	glm::vec4 fogDistance;
	glm::vec4 ambientColor;
	glm::vec4 sunlightDiretion;
	glm::vec4 sunlightColor;
};

struct UploadContext {
	VkFence _uploadFence;
	VkCommandPool _commandPool;
};

struct FrameData {
	VkSemaphore _presentSem, _renderSem;
	VkFence _renderFence;

	VkCommandPool _commandPool;
	VkCommandBuffer _commandBuffer;

};

struct AllFrameAllocatedBuffer
{
	AllocatedBuffer _cameraBuffer;
	AllocatedBuffer _objectBuffer;
	AllocatedBuffer _senneParameterBuffer;
};

struct Texture {
	AllocatedImage image;
	VkImageView imageView;
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
	VkPhysicalDeviceProperties _gpuProperties;
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

	FrameData _frames[FRAME_OVERLAP];

	VkQueue _graphicsQueue;
	uint32_t _graphicsQueueFamily;

	VkRenderPass _renderPass;
	std::vector<VkFramebuffer> _framebuffers;

	DeletionQueue _mainDeletionQueue;

	VmaAllocator _allocator;

	std::vector<uint16_t> shader_index;
	std::vector<std::string> obj_name;
	std::vector<std::string> shader_name;
	std::vector<std::string> texture_name;

	VkPipelineLayout _meshPipelineLayout;  // Temp var
	VkPipelineLayout _texturedPipeLayout;   // Temp var


	VkDescriptorSetLayout _globalSetLayout;
	VkDescriptorSetLayout _singleTextureSetLayout;
	VkDescriptorPool _descriptorPool;
	
	UploadContext _uploadContext;

	// All Frame use same Descriptor
	VkDescriptorSet _globalDescriptor;
	// all use same description set
	AllFrameAllocatedBuffer _all_allcated_buffer;
	// dynamic description set
	GPUSenceData _senceParameters;
	AllocatedBuffer _senneParameterBuffer;
	// define you need pipeline
	std::vector<RenderObject> _renderObject;
	std::unordered_map<std::string, Mesh> _meshSet;
	std::unordered_map<std::string, Material> _material;
	std::unordered_map<std::string, Texture> _loadedTextures;
	movestatus _movestatus;
	//initializes everything in the engine
	void init();

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();

	AllocatedBuffer create_buffer(
		size_t allocSize,
		VkBufferUsageFlags usage,
		VmaMemoryUsage memoryUsage
	);
	// Mesh Part
	void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
	Material* create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);
	Material* get_material(const std::string& name);
	Mesh* getMesh(const std::string& name);
	void draw_object(VkCommandBuffer cmd, RenderObject* first, int count);
	FrameData& get_current_frame();
	
	size_t pad_uniform_buffer_size(size_t originalSize);
	// Math
	void UpdateDate(int obj_indx);

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
	void load_images();
	void upload_mesh(Mesh& mesh);
	void key_event_process(int32_t keycode);
	void init_descriptors();
	void init_imgui();
};