#include "vk_utils.h"

int checkValidationLayerSupport(const char** validationLayers) {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, NULL);

  VkLayerProperties *available_layers =
      malloc((layer_count - 1) * sizeof(VkLayerProperties));
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

  for (int i = 0; i < sizeof(validationLayers) / sizeof(char *); i++) {
    const char *validation_layer = validationLayers[i];
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


int isDeviceSuitable(VkPhysicalDevice device) { // Read caller method in vk_setup.c
  /** VkPhysicalDeviceProperties device_properties;
  vkGetPhysicalDeviceProperties(device, &device_properties);
  VkPhysicalDeviceFeatures device_features;
  vkGetPhysicalDeviceFeatures(device, &device_features);

  return device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
    device_features.geometryShader;
    */
  VkPhysicalDeviceFeatures device_features;
  vkGetPhysicalDeviceFeatures(device, &device_features);


  struct QueueFamilyIndices indices = findQueueFamilies(device);
  if(!OPTIONAL_CHANGED(indices.graphics_family)) { return 0; }
  if(!device_features.geometryShader) {return 0;} // Minimal requirement

  return 1;
}



struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice dev) { // NOTE: Used together in isDeviceSuitable. Reference: https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
  struct QueueFamilyIndices indices;
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, NULL);

  VkQueueFamilyProperties* queue_families = GET_ARRAY(VkQueueFamilyProperties, queue_family_count-1);
  vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, queue_families);

  for(int i = 0; i < queue_family_count; i++) {
    if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      OPTIONAL_MODIFY(indices.graphics_family) = i; 
      break;
    }
  }

  free(queue_families);

  return indices;
}
