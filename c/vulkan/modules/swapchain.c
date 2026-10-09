#include <stdio.h>

#include "vk_swapchain_utils.h"
#include "vk_instance.h"
#include "swapchain.h"


static int impl_create_swapchain();
static int impl_create_surface();
static int impl_create_image_views();

struct swapchain_api swapchain_api = {
  .create_surface = impl_create_surface,
  .create_swapchain = impl_create_swapchain,
  .create_image_views = impl_create_image_views
};

static void __attribute((constructor)) init_api() {
  api.swapchain_api = swapchain_api;
}

static int impl_create_surface() {
  int res = glfwCreateWindowSurface(vk_inst.instance.vk_instance,
      vk_inst.instance.window,
      NULL,
      &vk_inst.graphics.swapchain_s.surface);
  if(res != VK_SUCCESS) {
    printf("Surface creation went wrong! Code: %d\n", res);
    return FAILURE;
  }
return SUCCESS;
}


static int impl_create_swapchain(){
  struct vk_utils_info_s swapchain_util_info = {
    .physical_device = vk_inst.devices.physical_device,
    .surface = vk_inst.graphics.swapchain_s.surface,
    .window = vk_inst.instance.window 
  };
  struct SwapChainSupportDetails swapchain_support;

  source_swapchain_utils(swapchain_util_info);
  querySwapChainSupport(&swapchain_support);

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(&swapchain_support);
  VkPresentModeKHR presentMode = chooseSwapPresentationMode(&swapchain_support);
  VkExtent2D extent = chooseSwapExtent(&swapchain_support);

  // Save up data for later
  vk_inst.graphics.swapchain_s.extent = extent;
  vk_inst.graphics.swapchain_s.image_format = surfaceFormat.format;

  uint32_t imageCount = swapchain_support.capabilities.minImageCount + 1;
       // Request one more to not get minimal, may sometimes be bottlenecked by driver's own procedures.
  if(swapchain_support.capabilities.maxImageCount > 0) { // If zero meaning no limits for max swap chain size.
    CLAMP(imageCount, swapchain_support.capabilities.minImageCount, swapchain_support.capabilities.maxImageCount);
  }

  VkSwapchainCreateInfoKHR create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = vk_inst.graphics.swapchain_s.surface;
  create_info.minImageCount = imageCount;
  create_info.imageFormat = surfaceFormat.format;
  create_info.imageColorSpace = surfaceFormat.colorSpace;
  create_info.imageExtent = extent; 
  create_info.imageArrayLayers = 1; // Always 1 if not "stereoscopic 3D" - VulkanTutorial
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Direct rendering on the swap.
  create_info.preTransform = swapchain_support.capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Ignore alpha channel to not interact with other windows.
  create_info.presentMode = presentMode;
  create_info.clipped = VK_TRUE; // Ignore overlapping color of pixels that are obscured by f.ex. another window.
  create_info.oldSwapchain = VK_NULL_HANDLE; // Ingore for now but TODO on resizing etc.

  uint32_t queue_family_indices[] = {vk_inst.devices.queue_families.graphics_family.value,
    vk_inst.devices.queue_families.present_family.value};

  if(vk_inst.devices.queue_families.graphics_family.value !=
      vk_inst.devices.queue_families.present_family.value){ // If the two families differenciate
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // No ownership of an image
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Exclusive for only one family at a time, best performance wise.
  }

  VkResult res = vkCreateSwapchainKHR(vk_inst.devices.device, &create_info, NULL, &vk_inst.graphics.swapchain_s.swapchain);
  swapchain_support.free(&swapchain_support);
  if(res != VK_SUCCESS) {
    printf("Failed to create a swapchain! Code: %d\n", res);
    return FAILURE;
  }

  // Our specification just describe the minimal amount of images, therefore retrieve whats actually been settled.
  vkGetSwapchainImagesKHR(vk_inst.devices.device, 
      vk_inst.graphics.swapchain_s.swapchain,
      &vk_inst.graphics.swapchain_s.swapchain_image_count,
      NULL);

  vk_inst.graphics.swapchain_s.p_swapchain_images = GET_ARRAY(VkImage, vk_inst.graphics.swapchain_s.swapchain_image_count);
  vkGetSwapchainImagesKHR(vk_inst.devices.device, 
      vk_inst.graphics.swapchain_s.swapchain,
      &vk_inst.graphics.swapchain_s.swapchain_image_count,
      vk_inst.graphics.swapchain_s.p_swapchain_images);

  return SUCCESS;
}

static int impl_create_image_views() {

  vk_inst.graphics.swapchain_s.p_swapchain_image_views = GET_ARRAY(VkImageView, vk_inst.graphics.swapchain_s.swapchain_image_count);

  for(int i = 0; i < vk_inst.graphics.swapchain_s.swapchain_image_count; i++){
    VkImageViewCreateInfo create_info={};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = vk_inst.graphics.swapchain_s.p_swapchain_images[i];
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // Image data interpretation
    create_info.format = vk_inst.graphics.swapchain_s.image_format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // Color channel mapping (this is default)
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Image purpose and how to access
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    
    VkResult res = vkCreateImageView(vk_inst.devices.device, &create_info, NULL, &vk_inst.graphics.swapchain_s.p_swapchain_image_views[i]);
    if(res != VK_SUCCESS) {
      printf("Failed to create image view(s)! Code: %d\n", res);
      return FAILURE;
    }
  }

  return SUCCESS;
}


