#include <stdio.h>
#define GLFW_INCLUDE_VULKAN
#include "vk_setup.h"

// Pawel Sapula
// Started 19.09.2026
// Following https://vulkan-tutorial.com for a good starting point (Amazing tutorial (learning resource) btw.)
// 01:24, 26.09.2026 - Hello World triangle

static void run();

int main() { run(); }

static void run() {

  if(!glfwInit()){
    printf("Failed to init GLFW!");
    goto exit;
  }

  if(create_glfw_window("GLFW Window") != SUCCESS) {goto exit;}
  if(vk_initialize() != SUCCESS) { goto exit; }
  if(vk_create_surface() != SUCCESS) {goto exit;}
  if(vk_pick_physical_device() != SUCCESS) {goto exit;}
  if(vk_create_logical_device() != SUCCESS) {goto exit;}
  if(vk_create_swapchain() != SUCCESS) {goto exit;}
  if(vk_create_image_views() != SUCCESS) {goto exit;}
  if(vk_create_render_pass() != SUCCESS) {goto exit;}
  if(vk_create_graphics_pipeline() != SUCCESS) {goto exit;}
  if(vk_create_framebuffers() != SUCCESS) {goto exit;}
  if(vk_create_command_pool() != SUCCESS) {goto exit;}
  if(vk_create_command_buffer() != SUCCESS) {goto exit;}
  if(vk_create_sync_objects() != SUCCESS) {goto exit;}

  vk_loop();
exit:
  vk_cleanup();
}

