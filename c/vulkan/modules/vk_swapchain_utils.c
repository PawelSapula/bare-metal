#include "vk_swapchain_utils.h"
#include <string.h>

static struct vk_utils_info_s info;

void source_swapchain_utils(struct vk_utils_info_s info_s) {
  info = info_s;
}

void querySwapChainSupport(struct SwapChainSupportDetails* details) {
  details->free = impl_free_swapChainSupportDetails; // Ready up free callback implementation immeadietly before anything can happen.

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(info.physical_device, info.surface, &details->capabilities);

  vkGetPhysicalDeviceSurfaceFormatsKHR(info.physical_device, info.surface, &details->formats_count, NULL);
  vkGetPhysicalDeviceSurfacePresentModesKHR(info.physical_device, info.surface, &details->present_modes_count, NULL);

  if (details->formats_count == 0 || details->present_modes_count == 0) {
    return; // Handled further by the isDeviceSuitable malloc check.
  }

  details->p_formats = GET_ARRAY(VkSurfaceFormatKHR, details->formats_count);
  details->p_present_modes = GET_ARRAY(VkPresentModeKHR, details->present_modes_count);
  VkSurfaceFormatKHR formats_prev[details->formats_count];
  VkPresentModeKHR present_modes_prev[details->present_modes_count];

  vkGetPhysicalDeviceSurfaceFormatsKHR(info.physical_device, info.surface, &details->formats_count, details->p_formats);
  vkGetPhysicalDeviceSurfacePresentModesKHR(info.physical_device, info.surface, &details->present_modes_count, details->p_present_modes);

  int res1 = memcmp(details->p_formats, formats_prev, sizeof(VkSurfaceFormatKHR)*details->formats_count);
  int res2 = memcmp(details->p_present_modes, present_modes_prev, sizeof(VkPresentModeKHR)*details->present_modes_count);

  if(res1 == 0 || res2 == 0) { // Memory not changed
    details->fail_flag = 1;
  }
  details->fail_flag = 0;

}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(struct SwapChainSupportDetails* details) {

  for(int i = 0; i < details->formats_count; i++) {
    if(details->p_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
        details->p_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
      return details->p_formats[i];
    }
  }

  return details->p_formats[0];
}

VkPresentModeKHR chooseSwapPresentationMode(struct SwapChainSupportDetails* details) {

  for(int i = 0; i < details->present_modes_count; i++) {
    if(details->p_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) { // Varation of FIFO, less latency problems since it replaces last existing image instead of making the program wait if that occurs.
      return details->p_present_modes[i];
    }
  }

  return  VK_PRESENT_MODE_FIFO_KHR; // Double swapchain buffer, similar to vertical synchronization (vsync).
}

VkExtent2D chooseSwapExtent(struct SwapChainSupportDetails* details) {
  if(details->capabilities.currentExtent.width != UINT32_MAX) {
    return details->capabilities.currentExtent;
  } // Window managers that cant specify the swapchain extend on the window normally set the value to UINT32_MAX

  int width, height;
  glfwGetFramebufferSize(info.window, &width, &height); // Get aspects in pixels since screen coords wont do the trick
                                                        // This comes from screen pixel density compared to the screen coords.

  VkExtent2D actual_extent = {
    (uint32_t)width,
    (uint32_t)height
  };

  actual_extent.width = CLAMP(actual_extent.width, details->capabilities.minImageExtent.width, details->capabilities.minImageExtent.width); 
  actual_extent.height = CLAMP(actual_extent.height, details->capabilities.minImageExtent.height, details->capabilities.minImageExtent.height); 
  return actual_extent;

}

