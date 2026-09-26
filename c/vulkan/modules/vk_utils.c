#include <string.h>

#include "vk_utils.h"
#include "vk_swapchain_utils.h"

static struct vk_utils_info_s info;

void source_vk_utils(struct vk_utils_info_s info_s) {
  info = info_s;
}

int checkValidationLayerSupport() {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, NULL);

  VkLayerProperties *available_layers =
      malloc((layer_count - 1) * sizeof(VkLayerProperties));
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

  for (int i = 0; i < sizeof(info.validation_layers) / sizeof(char *); i++) {
    const char *validation_layer = info.validation_layers[i];
    int found_layer = 0;

    for (int j = 0; j < layer_count; j++) {
      const VkLayerProperties available_layer = available_layers[j];
      if (strcmp(validation_layer, available_layer.layerName) == 0) {
        found_layer = 1;
        break;
      }
    }
    if (!found_layer) {
      free(available_layers);
      return 0;
    }
  }
  free(available_layers);
  return 1;
}

static int checkDeviceExtensionSupport(){
  uint32_t extension_count;
  vkEnumerateDeviceExtensionProperties(info.physical_device, NULL, &extension_count, NULL);

  VkExtensionProperties* available_extensions = GET_ARRAY(VkExtensionProperties, extension_count);
  vkEnumerateDeviceExtensionProperties(info.physical_device, NULL, &extension_count, available_extensions);

  int array_size = sizeof(info.device_extensions)/sizeof(info.device_extensions[0]);
  int found = 0;
  for(int i = 0; i < array_size; i++) {
    for(int j = 0; j < extension_count; j++) {
      if(strcmp(info.device_extensions[i], available_extensions[j].extensionName) == 0) {
        found = 1; // TODO: Change here if more device extensions are to be expected!
        break;
      }
    }
  }

  free(available_extensions);
  if(found == 1) {return 1;}
  return 0;
}


int isDeviceSuitable() { // Read caller method in vk_setup.c
  /** VkPhysicalDeviceProperties device_properties;
  vkGetPhysicalDeviceProperties(device, &device_properties);
  VkPhysicalDeviceFeatures device_features;
  vkGetPhysicalDeviceFeatures(device, &device_features);

  return device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
    device_features.geometryShader;
    */
  //VkPhysicalDeviceFeatures device_features;
  //vkGetPhysicalDeviceFeatures(device, &device_features);


  struct QueueFamilyIndices indices = findQueueFamilies();
  if(!OPTIONAL_CHANGED(indices.graphics_family)) { return 0; }
  if(!OPTIONAL_CHANGED(indices.present_family)) { return 0; }
  //if(!device_features.geometryShader) {return 0;} // Minimal requirement, too bad that M1 doesnt support geometry shaders.

  if(!checkDeviceExtensionSupport()) {return 0;} 
  else {
    struct SwapChainSupportDetails swapchain_support_details;
    source_swapchain_utils(info);
    querySwapChainSupport(&swapchain_support_details);
    if(!swapchain_support_details.p_formats || !swapchain_support_details.p_present_modes) {return 0;} // Quick unitialzied check
    swapchain_support_details.free(&swapchain_support_details); // Just a check for flag, dont need the actual info before creation.
    if(swapchain_support_details.fail_flag == 1) { 
      return 0; 
    }

  }

  return 1;
}

struct QueueFamilyIndices findQueueFamilies() { // NOTE: Used together in isDeviceSuitable. Reference: https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
  struct QueueFamilyIndices indices;
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(info.physical_device, &queue_family_count, NULL);

  VkQueueFamilyProperties* queue_families = GET_ARRAY(VkQueueFamilyProperties, queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(info.physical_device, &queue_family_count, queue_families);

  VkBool32 presentSupport = 0;

  for(int i = 0; i < queue_family_count; i++) {
    if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      OPTIONAL_MODIFY(indices.graphics_family) = i; 
    }
    vkGetPhysicalDeviceSurfaceSupportKHR(info.physical_device, i, info.surface, &presentSupport);
    if(presentSupport) {
      OPTIONAL_MODIFY(indices.present_family) = i;
    }
  }

  free(queue_families);

  return indices;
}
