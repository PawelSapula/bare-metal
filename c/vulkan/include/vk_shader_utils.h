#ifndef VK_SHADER_UTILS_H
#define VK_SHADER_UTILS_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vk_data.h"

static const char* VERT_SHADER_PATH = "shaders/vert.spv";
static const char* FRAG_SHADER_PATH = "shaders/frag.spv";

void source_shader_utils(struct vk_utils_info_s info_s);

VkShaderModule create_shader_module(const char* path);

#endif /* VK_SHADER_UTILS_H */
