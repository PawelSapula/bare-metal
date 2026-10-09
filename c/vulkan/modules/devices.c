#include <stdint.h>
#include <stdio.h>

#include "utils.h"
#include "vk_utils.h"
#include "vk_instance.h"
#include <devices.h>

static int impl_pick_physical_device();
static int impl_create_logical_device();

struct device_api device_api = {
  .create_logical_device = impl_create_logical_device,
  .pick_physical_device = impl_pick_physical_device
};

static void __attribute((constructor)) init_api() {
  api.device_api = device_api;
}

static int impl_pick_physical_device(){ // TODO: Chooses first device from the llist if its suitable. If project goes to heavier computation, add checks for found devices.
  uint32_t device_count = 0;    // See: vk_utils.h -> isDeviceSuitable
  vkEnumeratePhysicalDevices(vk_inst.instance.vk_instance, &device_count, NULL);
  
  if(device_count == 0) {
    printf("None GPU's available with Vulkan support!");
    return FAILURE;
  }

  VkPhysicalDevice* physical_devices = GET_ARRAY(VkPhysicalDevice, device_count);
  vkEnumeratePhysicalDevices(vk_inst.instance.vk_instance, &device_count, physical_devices);


  for(int i = 0; i < device_count; i++) {
    struct vk_utils_info_s utils_info = {
      .physical_device = physical_devices[i],
      .surface = vk_inst.graphics.swapchain_s.surface,
      .device_extensions = device_extensions
    }; // Update from loop to iterate over.
    source_vk_utils(utils_info);
    if(isDeviceSuitable()){
      vk_inst.devices.physical_device = physical_devices[i];
      break;
      }
  }

  if(vk_inst.devices.physical_device == VK_NULL_HANDLE) {
    printf("Failed to find a suitable GPU.");
    free(physical_devices);
    return FAILURE;
  }

  free(physical_devices);
  return SUCCESS;
}

int impl_create_logical_device() {
  struct vk_utils_info_s utils_info = {
    .physical_device = vk_inst.devices.physical_device,
    .surface = vk_inst.graphics.swapchain_s.surface 
  };
  source_vk_utils(utils_info);

  struct QueueFamilyIndices indices = findQueueFamilies();
  // Store indices for later use instead of calling this function. Primarly to decouple components.
  vk_inst.devices.queue_families.graphics_family = indices.graphics_family;
  vk_inst.devices.queue_families.present_family = indices.present_family;

  int32_t queue_count = 2; // TODO: Change from manual counting 
  VkDeviceQueueCreateInfo* queue_create_infos = GET_ARRAY(VkDeviceQueueCreateInfo, queue_count);
  uint32_t unique_queue_families[] = {indices.graphics_family.value, indices.present_family.value};

  float queue_priority = 1.0f;
  for(int i = 0; i < queue_count; i++){
    uint32_t queue_family = unique_queue_families[i];
    VkDeviceQueueCreateInfo queue_create_info = {}; // Specify the number of queues we want for a single queue family.
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family;
    queue_create_info.queueCount = 1;   // Typically use one because its a standard approach to multithread command buffers and submit them all at once.
    queue_create_info.pQueuePriorities = &queue_priority; // Scheduling priority. Required also when the queueCount == 1.

    queue_create_infos[i] = queue_create_info; // Append to array
  }

  VkPhysicalDeviceFeatures device_features = {}; // Specification of device features that we will be using. Empty for now (Passed physical device check earlier @isDeviceSuitable)

  VkDeviceCreateInfo create_info = {}; // Info structure for the actual logical device.
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.pQueueCreateInfos = queue_create_infos;
  create_info.queueCreateInfoCount = queue_count;
  create_info.pEnabledFeatures = &device_features;
  create_info.ppEnabledExtensionNames = device_extensions;
  create_info.enabledExtensionCount = sizeof(device_extensions)/sizeof(device_extensions[0]);

  int res = vkCreateDevice(vk_inst.devices.physical_device, &create_info, NULL, &vk_inst.devices.device);
  if(res != VK_SUCCESS) {
    printf("Logical device creation failed! Code: %d\n", res);
    free(queue_create_infos);
    return FAILURE;
  }

  vkGetDeviceQueue(vk_inst.devices.device, indices.graphics_family.value, 0, &vk_inst.graphics.swapchain_s.queues.graphics_queue);
  vkGetDeviceQueue(vk_inst.devices.device, indices.present_family.value, 0, &vk_inst.graphics.swapchain_s.queues.present_queue);

  free(queue_create_infos);
  return SUCCESS;

}

