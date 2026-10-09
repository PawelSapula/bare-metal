#ifndef VK_INSTANCE_H
#define VK_INSTANCE_H
#include "buffers.h"
#include "draw.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "swapchain.h"
#include "devices.h"
#include "pipeline.h"


#ifdef NDEBUG
#define VALIDATION_LAYERS 0
#else
#define VALIDATIONS_LAYERS 1
#endif

#define WIN_SIZE_X 1280
#define WIN_SIZE_Y 1080

// Validation layers
static const char *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
static const char* device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME}; // Not all GPU's have swapchain support, ex. server GPU's.


struct api {
  int (*create_glfw_window)(const char* name);
  int (*initialize)();
  struct device_api device_api;
  struct swapchain_api swapchain_api;
  struct pipeline_api pipeline_api;
  struct buffer_api buffer_api;
  struct draw_api draw_api;
  void (*program_loop)();
  int (*cleanup)();
} api;

struct instance_s {
  GLFWwindow* window;
  VkInstance vk_instance;
};

struct queue_families {
  uint32_t_opt graphics_family;
  uint32_t_opt present_family; // Not all queue families supporting drawing commands support presentation.
};

struct devices_s {
  VkPhysicalDevice physical_device;
  VkDevice device;
  struct queue_families queue_families;
};

struct graphics_s {
  struct swapchain_s swapchain_s;
  VkRenderPass render_pass;
  VkPipelineLayout pipeline_layout;
  VkPipeline graphics_pipeline;
  VkCommandPool command_pool;
  VkCommandBuffer command_buffer; // Freed automatically with the pool
};

struct vk_instance {
  struct instance_s instance;
  struct devices_s devices;
  struct graphics_s graphics;
} vk_inst;

void register_api(void (*set_field)(struct api api));

#endif /* VK_INSTANCE_H */
