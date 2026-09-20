#include "vulkan_init.h"

GLFWwindow* create_glfw_window(const char* window_title){
  GLFWwindow* window = glfwCreateWindow(WIN_SIZE_X, WIN_SIZE_Y, window_title, NULL, NULL);
  if(!window) {
    printf("Window initialization went wrong!");
  }
  return window;
}
