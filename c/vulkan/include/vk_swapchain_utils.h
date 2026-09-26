#ifndef VK_SWAPCHAIN_UTILS_H
#define VK_SWAPCHAIN_UTILS_H

#include "vk_data.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdlib.h>

struct SwapChainSupportDetails { // Check whats available to use for the swapchain, because not necessary available with our window surface.
  VkSurfaceCapabilitiesKHR capabilities;
  VkSurfaceFormatKHR* p_formats;
  uint32_t formats_count;
  VkPresentModeKHR* p_present_modes;
  uint32_t present_modes_count;
  int8_t fail_flag; // Only relevant in @isDeviceSuitable

  void (*free)(struct SwapChainSupportDetails* const pself_SwapChainSupportDetails);
};

static void impl_free_swapChainSupportDetails(struct SwapChainSupportDetails* const pself_SwapChainSupportDetails) { // Method to not manually free the struct, eventually reuse.
  free(pself_SwapChainSupportDetails->p_formats);
  free(pself_SwapChainSupportDetails->p_present_modes);
  pself_SwapChainSupportDetails->p_formats = 0;
  pself_SwapChainSupportDetails->p_present_modes = 0;
  pself_SwapChainSupportDetails->formats_count = 0;
  pself_SwapChainSupportDetails->present_modes_count = 0;
}

/**
 * @brief Function to send data for use by the different methods.
 * @param vk_utils_info_s
 */
void source_swapchain_utils(struct vk_utils_info_s info_s);

// @Req: physical_device, surface
void querySwapChainSupport(struct SwapChainSupportDetails* details);

/////////// Methods for calibrating swapchain settings ////////////////
///TODO: Change to local copies instead of pointer ref? Maybe keep for speed.
VkSurfaceFormatKHR chooseSwapSurfaceFormat(struct SwapChainSupportDetails* details);
VkPresentModeKHR chooseSwapPresentationMode(struct SwapChainSupportDetails* details);
//Req: window 
VkExtent2D chooseSwapExtent(struct SwapChainSupportDetails* details);
///////////////////////////////////////////////////////////////////////

#endif /* VK_SWAPCHAIN_UTILS_H */
