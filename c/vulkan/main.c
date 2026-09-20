#define GLFW_INCLUDE_VULKAN
#include "modules/vulkan_init.h"
#include <GLFW/glfw3.h>

#define GLFW_FORCE_RADIANS
#define GLFW_FORCE_DEPTH_ZERO_TO_ONE

int main() {
  glfwInit();

  GLFWwindow* window = create_glfw_window("GLFW window");

   uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

  printf("%d\n extensions supported", extensionCount);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
  
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
