// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include <vk_mesh.h>

struct AllocatedImage {
	VkImage _image;
	VmaAllocation _allocation;
};
//we will add our main reusable types here