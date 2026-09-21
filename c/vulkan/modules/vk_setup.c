#include "vk_setup.h"
#include "vk_utils.h"
#include <stdio.h>
#include <vulkan/vulkan_core.h>


static GLFWwindow *window;
static VkInstance instance;
static VkPhysicalDevice physical_device = VK_NULL_HANDLE; // Frees automatically on instance destruction
static VkDevice device;
static VkSurfaceKHR surface; // A abstract surface that connects via. WSI (Window Subsystem Integration) extention (given by glfwGetReq..) to connect with the glfw window.
                             // System dependent but glfw handles it via a system type extension through the same function as over.

struct queues {
  VkQueue graphics_queue;
} static queues;

// Validation layers
static const char *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};


int create_glfw_window(const char* window_title){
  window = glfwCreateWindow(WIN_SIZE_X, WIN_SIZE_Y, window_title, NULL, NULL);
  if(!window) {
    printf("Window initialization went wrong!");
    return FAILURE;
  }
  return SUCCESS;
}

int vk_initialize() {
  VkApplicationInfo app_info = {};
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No engine";
  app_info.pApplicationName = "Vulkan";
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

  uint32_t glfw_extensionCount = 0; // Find extensions with the window systems
                                    // bcuz Vulkan is platmorm agnistic API
  const char **glfw_extensions;
  glfwGetRequiredInstanceExtensions(&glfw_extensionCount);

  // MacOs relevant to solve for Metal
  const char **required_extensions =
      malloc(glfw_extensionCount * sizeof(char *));
  if (!required_extensions) {
  }
  for (int i = 0; i < glfw_extensionCount; i++) {
    required_extensions[i] = glfw_extensions[i];
  }
  required_extensions[glfw_extensionCount] =
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
  //


  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledLayerCount = 0; // Refers to validation layers
  create_info.enabledExtensionCount =
      glfw_extensionCount +
      1; // Determines what global validation layers to enable.
  create_info.ppEnabledExtensionNames = required_extensions; // This one too
  create_info.flags |=
      VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR; // MacOS relevant to
                                                        // solve for Metal
  if(VALIDATIONS_LAYERS && !checkValidationLayerSupport(validation_layers)) {
    printf("Validation layers requested, but not available.");
  }
  else{
    create_info.enabledLayerCount = (int32_t)(sizeof(validation_layers)/sizeof(char*));
    create_info.ppEnabledLayerNames = (const char**)validation_layers;
  }

  VkResult res = vkCreateInstance(&create_info, NULL, &instance) != VK_SUCCESS;
  if (res != VK_SUCCESS) {
    printf("Vulkan instance creation went wrong! Code: (%d)\n", res);
    return FAILURE;
  }

  free(required_extensions);
  return SUCCESS;
}

int vk_pick_physical_device(){ // TODO: Chooses first device from the llist if its suitable. If project goes to heavier computation, add checks for found devices.
  uint32_t device_count = 0;    // See: vk?utils.h -> isDeviceSuitable
  vkEnumeratePhysicalDevices(instance, &device_count, NULL);
  
  if(device_count == 0) {
    printf("None GPU's available with Vulkan support!");
    return FAILURE;
  }

  VkPhysicalDevice* physical_devices = GET_ARRAY(VkPhysicalDevice, device_count-1);
  vkEnumeratePhysicalDevices(instance, &device_count, physical_devices);

  for(int i = 0; i < device_count; i++) {
    if(isDeviceSuitable(physical_devices[i])){
      physical_device = physical_devices[i];
      break;
      }
  }

  if(physical_device == VK_NULL_HANDLE) {
    printf("Failed to find a suitable GPU.");
    return FAILURE;
  }

  free(physical_devices);
  return SUCCESS;
}

int vk_create_logical_device() {
  struct QueueFamilyIndices indices = findQueueFamilies(physical_device);

  VkDeviceQueueCreateInfo queue_create_info = {}; // Specify the number of queues we want for a single queue family.
  queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_info.queueFamilyIndex = indices.graphics_family.value;
  queue_create_info.queueCount = 1;   // Typically use one because its a standard approach to multithread command buffers and submit them all at once.

  float queue_priority = 1.0f;
  queue_create_info.pQueuePriorities = &queue_priority; // Scheduling priority. Required also when the queueCount == 1.

  VkPhysicalDeviceFeatures device_features = {}; // Specification of device features that we will be using. Empty for now (Passed physical device check earlier @isDeviceSuitable)

  VkDeviceCreateInfo create_info = {}; // Info structure for the actual logical device.
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.pQueueCreateInfos = &queue_create_info;
  create_info.queueCreateInfoCount = 1;
  create_info.pEnabledFeatures = &device_features;

  int res = vkCreateDevice(physical_device, &create_info, NULL, &device);
  if(res != VK_SUCCESS) {
    printf("Logical device creation failed! Code: %d\n", res);
    return FAILURE;
  }

  vkGetDeviceQueue(device, indices.graphics_family.value, 0, &queues.graphics_queue);
  return SUCCESS;

}

int vk_create_surface() {
  if(glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS) {
    printf("Surface creation went wrong!"); // ADD: Code result
    return FAILURE;
  }

  return SUCCESS;
}

void vk_loop() {

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
}

int vk_cleanup() {
  printf("Terminating...\n");
  vkDestroyDevice(device, NULL);
  vkDestroySurfaceKHR(instance, surface, NULL);
  vkDestroyInstance(instance, NULL);
  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
