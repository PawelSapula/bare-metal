#ifndef VK_UTILS_H_
#define VK_UTILS_H_
#include "vk_data.h"
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdlib.h>

/**
 * @brief Function to send data for use by the different methods.
 * @param vk_utils_info_s
 */
void source_vk_utils(struct vk_utils_info_s info_s);

// @Req: validation_layers
int checkValidationLayerSupport();

// @Req: device_extensions
//int checkDeviceExtensionSupport(); <- Static implementation now

// @Req: physical_device, surface, device_extensions (dep. checkDeviceExtensionSupport, vk_swapchain_utils->querySwapChainSupport)
int isDeviceSuitable();

// @Req: physical_device, surface
struct QueueFamilyIndices findQueueFamilies(); 

#endif /* VK_UTILS_H_ */
