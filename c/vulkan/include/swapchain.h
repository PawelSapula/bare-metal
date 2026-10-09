#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "utils.h"

struct swapchain_api {
 int (*create_surface)();
 int(*create_swapchain)();
 int(*create_image_views)();
};

struct queues_s {
  VkQueue graphics_queue;
  uint32_t_opt graphics_family_index;
  VkQueue present_queue;
  uint32_t_opt present_family_index; // Not all queue families supporting drawing commands support presentation.
};

struct swapchain_s {
  VkSwapchainKHR swapchain;
  VkSurfaceKHR surface;
  struct queues_s queues;
  VkFormat image_format;
  VkExtent2D extent;
  VkImage* p_swapchain_images;
  uint32_t swapchain_image_count;
  VkImageView* p_swapchain_image_views;
  VkFramebuffer* p_framebuffers;
};

#endif /* SWAPCHAIN_H */

