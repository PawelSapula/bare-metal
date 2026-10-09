#include "vk_instance.h"
#include "vk_utils.h"

static int impl_create_glfw_window(const char* name);
static int impl_initialize();
static void impl_program_loop();
static int impl_cleanup();

static void __attribute__((constructor)) init_api() {
  api.create_glfw_window = impl_create_glfw_window;
  api.initialize = impl_initialize;
  api.program_loop = impl_program_loop;
  api.cleanup = impl_cleanup;
}

static int impl_create_glfw_window(const char* name){
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Disable api initialization, crashed when trying to create a surface cuz glfw automatically setups for OpenGl.
  vk_inst.instance.window = glfwCreateWindow(WIN_SIZE_X, WIN_SIZE_Y, name, NULL, NULL);
  if(!vk_inst.instance.window) {
    printf("Window initialization went wrong!");
    return FAILURE;
  }
  return SUCCESS;
}

static int impl_initialize() {
  VkApplicationInfo app_info = {};
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No engine";
  app_info.pApplicationName = "Vulkan";
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

  uint32_t glfw_extensionCount = 0; // Find extensions with the window systems
                                    // bcuz Vulkan is platmorm agnistic API
  const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensionCount);
  if(glfw_extensions == NULL){
    printf("Failed to obtain required instance extensions through GLFW\n");
  }

  // MacOs relevant to solve for Metal
#ifdef TARGET_OS_MAC
  const char **required_extensions =
      malloc((glfw_extensionCount + 1) * sizeof(char *));
#else
  const char **required_extensions =
      malloc(glfw_extensionCount * sizeof(char *));
#endif
  for (int i = 0; i < glfw_extensionCount; i++) {
    required_extensions[i] = glfw_extensions[i];
  }
#ifdef TARGET_OS_MAC
  required_extensions[glfw_extensionCount] =
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
#endif

  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledLayerCount = 0; // Refers to validation layers
  create_info.enabledExtensionCount =
      glfw_extensionCount +
      1; // Determines what global validation layers to enable.
  create_info.ppEnabledExtensionNames = required_extensions; // This one too
                                                             //
  #ifdef TARGET_OS_MAC
  create_info.flags |=
      VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR; // MacOS relevant to
                                                        // solve for Metal
  #endif

  struct vk_utils_info_s utils_info = {.validation_layers = validation_layers};
  source_vk_utils(utils_info);
  if(VALIDATIONS_LAYERS && !checkValidationLayerSupport()) {
    printf("Validation layers requested, but not available.");
  }
  else{
    create_info.enabledLayerCount = (int32_t)(sizeof(validation_layers)/sizeof(char*));
    create_info.ppEnabledLayerNames = (const char**)validation_layers;
  }

  VkResult res = vkCreateInstance(&create_info, NULL, &vk_inst.instance.vk_instance) != VK_SUCCESS;
  if (res != VK_SUCCESS) {
    printf("Vulkan instance creation went wrong! Code: (%d)\n", res);
    free(required_extensions);
    return FAILURE;
  }

  free(required_extensions);
  return SUCCESS;
}

void impl_program_loop() {

  while (!glfwWindowShouldClose(vk_inst.instance.window)) {
    glfwPollEvents();
    api.draw_api.draw_frame();
  }
  vkDeviceWaitIdle(vk_inst.devices.device);
}

int impl_cleanup() {
  printf("Terminating...\n");

  // DESTRUCTION PRE L-DEVICE //
  api.draw_api.destroy_sync_objects();
  vkDestroyCommandPool(vk_inst.devices.device, vk_inst.graphics.command_pool, NULL);
  for(int i = 0; i < vk_inst.graphics.swapchain_s.swapchain_image_count; i++) {
    vkDestroyFramebuffer(vk_inst.devices.device, vk_inst.graphics.swapchain_s.p_framebuffers[i], NULL);
  }
  vkDestroyPipeline(vk_inst.devices.device, vk_inst.graphics.graphics_pipeline, NULL);
  vkDestroyPipelineLayout(vk_inst.devices.device, vk_inst.graphics.pipeline_layout, NULL);
  vkDestroyRenderPass(vk_inst.devices.device, vk_inst.graphics.render_pass, NULL);
  vkDestroySwapchainKHR(vk_inst.devices.device, vk_inst.graphics.swapchain_s.swapchain, NULL);
  for(int i = 0; i < vk_inst.graphics.swapchain_s.swapchain_image_count; i++) {
    vkDestroyImageView(vk_inst.devices.device, vk_inst.graphics.swapchain_s.p_swapchain_image_views[i], NULL);
  }
  free(vk_inst.graphics.swapchain_s.p_swapchain_image_views);
  free(vk_inst.graphics.swapchain_s.p_swapchain_images);
  free(vk_inst.graphics.swapchain_s.p_framebuffers);
  /////////////////////////////

  vkDestroyDevice(vk_inst.devices.device, NULL);
  vkDestroySurfaceKHR(vk_inst.instance.vk_instance, vk_inst.graphics.swapchain_s.surface, NULL);
  vkDestroyInstance(vk_inst.instance.vk_instance, NULL);
  glfwDestroyWindow(vk_inst.instance.window);
  glfwTerminate();
  return EXIT_SUCCESS;
}

