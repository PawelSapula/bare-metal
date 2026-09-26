#ifndef VK_SETUP_H_
#define VK_SETUP_H_
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <utils.h>
#include <vk_utils.h>

#include <stdlib.h>
#include <stdio.h>

#define WIN_SIZE_X 1280
#define WIN_SIZE_Y 1080

///////////////////// VARIABLES ////////////////////////


////////////////////////////////////////////////////////


///////////////// FUNCTION DECLARATIONS ////////////////


int vk_initialize();
int vk_create_surface();
int vk_pick_physical_device();
int vk_create_logical_device();
int vk_create_swapchain();
int vk_create_image_views();
int vk_create_render_pass();
int vk_create_graphics_pipeline();
int vk_create_framebuffers();
int vk_create_command_pool();
int vk_create_command_buffer();
int vk_create_sync_objects();
void vk_loop();
int vk_cleanup();
int create_glfw_window(const char* window_title);

////////////////////////////////////////////////////////

#endif /** VK_SETUP_H_ */
