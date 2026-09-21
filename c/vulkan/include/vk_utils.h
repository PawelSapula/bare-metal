#ifndef VK_UTILS_H_
#define VK_UTILS_H_
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <string.h>
#include "utils.h"

struct QueueFamilyIndices { // Btw indices is the plural form of an index.
  uint32_t_opt graphics_family;
  uint32_t_opt presentFamily; // Not all queue families supporting drawing commands support presentation.
};

int checkValidationLayerSupport(const char** validationLayers);
int isDeviceSuitable(VkPhysicalDevice device);
struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice dev);

#endif /* VK_UTILS_H_ */
