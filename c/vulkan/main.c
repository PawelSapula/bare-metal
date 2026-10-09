#include "vk_instance.h"
#include <assert.h>
#include <stdio.h>
#define GLFW_INCLUDE_VULKAN
#include "mystd/mystd.h"

// Pawel Sapula
// Started 19.09.2026
// Following https://vulkan-tutorial.com for a good starting point (Amazing tutorial (learning resource) btw.)
// 01:24, 26.09.2026 - Hello World triangle

static void run();

int main() { run(); }


static void run() {

  mystd_tests();

  if(!glfwInit()){
    printf("Failed to init GLFW!");
    goto exit;
  }

  if(api.create_glfw_window("GLFW Window") != SUCCESS) {goto exit;}
  if(api.initialize() != SUCCESS) { goto exit; }
  if(api.swapchain_api.create_surface() != SUCCESS) {goto exit;}
  if(api.device_api.pick_physical_device() != SUCCESS) {goto exit;}
  if(api.device_api.create_logical_device() != SUCCESS) {goto exit;}
  if(api.swapchain_api.create_swapchain() != SUCCESS) {goto exit;}
  if(api.swapchain_api.create_image_views() != SUCCESS) {goto exit;}
  if(api.pipeline_api.create_render_pass() != SUCCESS) {goto exit;}
  if(api.pipeline_api.create_graphics_pipeline() != SUCCESS) {goto exit;}
  if(api.buffer_api.create_frambuffers() != SUCCESS) {goto exit;}
  if(api.buffer_api.create_command_pool() != SUCCESS) {goto exit;}
  if(api.buffer_api.create_command_buffer() != SUCCESS) {goto exit;}
  if(api.draw_api.create_sync_objects() != SUCCESS) {goto exit;}

  api.program_loop(); 
exit:
  api.cleanup();
  return;
}

