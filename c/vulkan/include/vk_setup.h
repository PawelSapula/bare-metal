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
int vk_pick_physical_device();
int vk_create_logical_device();
void vk_loop();
int vk_cleanup();
int create_glfw_window(const char* window_title);

////////////////////////////////////////////////////////

#endif /** VK_SETUP_H_ */
