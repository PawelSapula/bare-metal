#ifndef VK_DATA_H
#define VK_DATA_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "utils.h"


struct QueueFamilyIndices { // Btw indices is the plural form of an index.
  uint32_t_opt graphics_family;
  uint32_t_opt present_family; // Not all queue families supporting drawing commands support presentation.
};


struct vk_utils_info_s {
  GLFWwindow* window;
  VkPhysicalDevice physical_device;
  VkDevice device;
  VkSurfaceKHR surface;
  const char** validation_layers;
  const char** device_extensions;
};
#endif /* VK_DATA_H */
