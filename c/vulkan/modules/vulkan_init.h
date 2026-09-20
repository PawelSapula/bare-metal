#ifndef VULKAN_INIT
#define VULKAN INIT
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>

#define WIN_SIZE_X 1280
#define WIN_SIZE_Y 1080

GLFWwindow* create_glfw_window(const char* window_title);

#endif /** VULKAN_INIT */
