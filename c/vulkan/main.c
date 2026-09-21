#include <stdio.h>
#define GLFW_INCLUDE_VULKAN
#include "vk_setup.h"

// Pawel Sapula
// Started 19.09.2026
// Following https://vulkan-tutorial.com for a good starting point (Amazing tutorial btw.)

static void run();

int main() { run(); }

static void run() {
  if(vk_initialize() != SUCCESS) { goto exit; }
  if(vk_pick_physical_device() != SUCCESS) {goto exit;}
  if(vk_create_logical_device() != SUCCESS) {goto exit;}

  glfwInit();
  if(create_glfw_window("GLFW Window") != SUCCESS) {goto exit;}
  vk_loop();
exit:
  vk_cleanup();
}

