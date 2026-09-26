#include "vk_setup.h"
#include "utils.h"
#include "vk_shader_utils.h"
#include "vk_utils.h"
#include "vk_swapchain_utils.h"
#include <stdint.h>
#include <stdio.h>
#include <vulkan/vulkan_core.h>

static GLFWwindow *window;
static VkInstance instance;
static VkPhysicalDevice physical_device = VK_NULL_HANDLE; // Frees automatically on instance destruction
static VkDevice device;
static VkSurfaceKHR surface; // A abstract surface that connects via. WSI (Window Subsystem Integration) extention (given by glfwGetReq..) to connect with the glfw window.
                             // System dependent but glfw handles it via a system type extension through the same function as over.
static VkRenderPass render_pass;
static VkPipelineLayout pipeline_layout;
static VkPipeline graphics_pipeline;
static VkCommandPool command_pool;
static VkCommandBuffer command_buffer; // Freed automatically with the pool

struct swapchain {
  VkSwapchainKHR swapchain;
  VkFormat image_format;
  VkExtent2D extent;
  VkImage* p_swapchain_images;
  uint32_t swapchain_image_count;
  VkImageView* p_swapchain_image_views;
  VkFramebuffer* p_framebuffers;
} static swapchain;

struct queues {
  VkQueue graphics_queue;
  VkQueue present_queue;
} static queues;

// Validation layers
static const char *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
// Device extensions TODO: On change look vk_utils.c
static const char* device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME}; // Not all GPU's have swapchain support, ex. server GPU's.


int create_glfw_window(const char* window_title){
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Disable api initialization, crashed when trying to create a surface cuz glfw automatically setups for OpenGl.
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

  VkResult res = vkCreateInstance(&create_info, NULL, &instance) != VK_SUCCESS;
  if (res != VK_SUCCESS) {
    printf("Vulkan instance creation went wrong! Code: (%d)\n", res);
    free(required_extensions);
    return FAILURE;
  }

  free(required_extensions);
  return SUCCESS;
}

int vk_create_surface() {
  int res = glfwCreateWindowSurface(instance, window, NULL, &surface);
  if(res != VK_SUCCESS) {
    printf("Surface creation went wrong! Code: %d\n", res);
    return FAILURE;
  }
return SUCCESS;
}

int vk_pick_physical_device(){ // TODO: Chooses first device from the llist if its suitable. If project goes to heavier computation, add checks for found devices.
  uint32_t device_count = 0;    // See: vk_utils.h -> isDeviceSuitable
  vkEnumeratePhysicalDevices(instance, &device_count, NULL);
  
  if(device_count == 0) {
    printf("None GPU's available with Vulkan support!");
    return FAILURE;
  }

  VkPhysicalDevice* physical_devices = GET_ARRAY(VkPhysicalDevice, device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, physical_devices);


  for(int i = 0; i < device_count; i++) {
    struct vk_utils_info_s utils_info = {
      .physical_device = physical_devices[i],
      .surface = surface,
      .device_extensions = device_extensions
    }; // Update from loop to iterate over.
    source_vk_utils(utils_info);
    if(isDeviceSuitable()){
      physical_device = physical_devices[i];
      break;
      }
  }

  if(physical_device == VK_NULL_HANDLE) {
    printf("Failed to find a suitable GPU.");
    free(physical_devices);
    return FAILURE;
  }

  free(physical_devices);
  return SUCCESS;
}

int vk_create_logical_device() {
  struct vk_utils_info_s utils_info = {.physical_device = physical_device, .surface = surface};
  source_vk_utils(utils_info);

  struct QueueFamilyIndices indices = findQueueFamilies();

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

  int res = vkCreateDevice(physical_device, &create_info, NULL, &device);
  if(res != VK_SUCCESS) {
    printf("Logical device creation failed! Code: %d\n", res);
    free(queue_create_infos);
    return FAILURE;
  }

  vkGetDeviceQueue(device, indices.graphics_family.value, 0, &queues.graphics_queue);
  vkGetDeviceQueue(device, indices.present_family.value, 0, &queues.present_queue);

  free(queue_create_infos);
  return SUCCESS;

}

int vk_create_swapchain(){
  struct vk_utils_info_s swapchain_util_info = {
    .physical_device = physical_device,
    .surface = surface,
    .window = window
  };
  struct SwapChainSupportDetails swapchain_support;

  source_swapchain_utils(swapchain_util_info);
  querySwapChainSupport(&swapchain_support);

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(&swapchain_support);
  VkPresentModeKHR presentMode = chooseSwapPresentationMode(&swapchain_support);
  VkExtent2D extent = chooseSwapExtent(&swapchain_support);

  // Save up data for later
  swapchain.extent = extent;
  swapchain.image_format = surfaceFormat.format;

  uint32_t imageCount = swapchain_support.capabilities.minImageCount + 1;
       // Request one more to not get minimal, may sometimes be bottlenecked by driver's own procedures.
  if(swapchain_support.capabilities.maxImageCount > 0) { // If zero meaning no limits for max swap chain size.
    CLAMP(imageCount, swapchain_support.capabilities.minImageCount, swapchain_support.capabilities.maxImageCount);
  }

  VkSwapchainCreateInfoKHR create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = surface;
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

  /** 
   * Here we specify how swap chain images will be handeled across multiple families if thats the case.
   * 
   * IMO, i think its a bad way to do this because instead of basing us the already defined queue families from the
   * logical device. This is potentially handled with the creation of the logical device, that references directly the physical
   * device. This should be checked out on some spare time. For now it works :) TODO
   */
  source_vk_utils(swapchain_util_info); 
  struct QueueFamilyIndices indices = findQueueFamilies();
  uint32_t queue_family_indices[] = {indices.graphics_family.value, indices.present_family.value};

  if(indices.graphics_family.value != indices.present_family.value){ // If the two families differenciate
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // No ownership of an image
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Exclusive for only one family at a time, best performance wise.
  }

  VkResult res = vkCreateSwapchainKHR(device, &create_info, NULL, &swapchain.swapchain);
  swapchain_support.free(&swapchain_support);
  if(res != VK_SUCCESS) {
    printf("Failed to create a swapchain! Code: %d\n", res);
    return FAILURE;
  }

  // Our specification just describe the minimal amount of images, therefore retrieve whats actually been settled.
  vkGetSwapchainImagesKHR(device, swapchain.swapchain, &swapchain.swapchain_image_count, NULL);
  swapchain.p_swapchain_images = GET_ARRAY(VkImage, swapchain.swapchain_image_count);
  vkGetSwapchainImagesKHR(device, swapchain.swapchain, &swapchain.swapchain_image_count, swapchain.p_swapchain_images);

  return SUCCESS;
}

// Tell Vulkan about the framebuffer attachments that are going to be used while rendering.
int vk_create_render_pass() {
 VkAttachmentDescription color_attachment = {}; 
 color_attachment.format = swapchain.image_format;
 color_attachment.samples = VK_SAMPLE_COUNT_1_BIT; // Nothing to multisample yet (check vk_create_views) TODO
 color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear the values to a constant at the start
 color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Rendered contents wil lbe stored in memory and can be read later.
 color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
 color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // Stencil buffer, ignore for now
 color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Ignore previous layout of the image.
 color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Images to be presented to the swapchain
                                                                 //   Many other common for final layout.
 VkAttachmentReference color_attachment_reference = {};
 color_attachment_reference.attachment = 0; // Refference to the attachment over ˆˆˆ. We have only one so it's index will be 0.
 color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

// Render passes can have multiple subpasses
// Each subpass references one or more attachments
 VkSubpassDescription subpass = {};
 subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Specify that this is a subpass for graphics.
 subpass.colorAttachmentCount = 1;
 subpass.pColorAttachments = &color_attachment_reference;

 VkRenderPassCreateInfo render_pass_info = {};
 render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
 render_pass_info.attachmentCount = 1;
 render_pass_info.pAttachments = &color_attachment;
 render_pass_info.subpassCount = 1;
 render_pass_info.pSubpasses = &subpass;

 // Synchronization for the subpass
 VkSubpassDependency dependency = {};
 dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Refers to subpass before or after the render paass
 dependency.dstSubpass = 0; // Refers to our subpass
 dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Waiting for color attachment output stage
 dependency.srcAccessMask = 0;
 dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; //  Wait until color attachment state and involve writing of the color attachment.
 dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;// Waiting for color attachment output stage

 render_pass_info.dependencyCount = 1;
 render_pass_info.pDependencies = &dependency;

 VkResult res = vkCreateRenderPass(device, &render_pass_info, NULL, &render_pass);
 if(res != VK_SUCCESS) {
    printf("Failed to create render pass! Code: %d\n", res);
    return FAILURE;
 }
 return SUCCESS;

}

int vk_create_image_views() {

  swapchain.p_swapchain_image_views = GET_ARRAY(VkImageView, swapchain.swapchain_image_count);

  for(int i = 0; i < swapchain.swapchain_image_count; i++){
    VkImageViewCreateInfo create_info={};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = swapchain.p_swapchain_images[i];
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // Image data interpretation
    create_info.format = swapchain.image_format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // Color channel mapping (this is default)
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Image purpose and how to access
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    
    VkResult res = vkCreateImageView(device, &create_info, NULL, &swapchain.p_swapchain_image_views[i]);
    if(res != VK_SUCCESS) {
      printf("Failed to create image view(s)! Code: %d\n", res);
      return FAILURE;
    }
  }

  return SUCCESS;
}

int vk_create_graphics_pipeline(){
  source_shader_utils((struct vk_utils_info_s){.device = device});
 VkShaderModule vertex_shader_module = create_shader_module(VERT_SHADER_PATH);
 VkShaderModule fragment_shader_module = create_shader_module(FRAG_SHADER_PATH);

 if(vertex_shader_module == NULL || fragment_shader_module == NULL){
  return FAILURE;
 }

 VkPipelineShaderStageCreateInfo vertex_shader_stage_info = {};
 vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
 vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
 vertex_shader_stage_info.module = vertex_shader_module;
 vertex_shader_stage_info.pName = "main";
 vertex_shader_stage_info.pSpecializationInfo = NULL; // Specify values for shader constants so that behaviour can be configured efficiently on pipeline creation.
                                                      
 VkPipelineShaderStageCreateInfo  fragment_shader_stage_info = {};
 fragment_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
 fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
 fragment_shader_stage_info.module = fragment_shader_module;
 fragment_shader_stage_info.pName = "main";

 VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_shader_stage_info, fragment_shader_stage_info};

 // Vertex input stage 
 // Bindings:                 Spacing between data and whether its per vertex or per instance
 // Attribute descriptions:   Types of the attributes passed to the vertex shader, which binding to load them from and at which offset.
 // For now: Nulled out because the shaders have all vertices data. TODO
 VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
 vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
 vertex_input_info.vertexBindingDescriptionCount = 0;
 vertex_input_info.pVertexBindingDescriptions = NULL;
 vertex_input_info.vertexAttributeDescriptionCount = 0;
 vertex_input_info.pVertexAttributeDescriptions = NULL;

 // Input assembly stage
 // Describes two twings:
 //   - What kind of geometry will be drawn from the vertices (topology) VK_PRIMITIVE_TOPOLOGY_...
 //   - Primitive restart [enabled or not]
 VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
 input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
 input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
 input_assembly_info.primitiveRestartEnable = VK_FALSE; // Enables to break up lines and traingles in the _STRIP topology mode(s).

 VkViewport viewport = {}; // Define the transformation from the image to the framebuffer.
 viewport.x = 0.0f;
 viewport.y = 0.0f;
 viewport.width = (float)swapchain.extent.width;
 viewport.height = (float)swapchain.extent.height;
 viewport.minDepth = 0.0f;
 viewport.maxDepth = 1.0f;

 VkRect2D scissor = {}; // Define where pixels will actually be stored (outside are disregarded by the rasterizer)
 scissor.offset = (VkOffset2D){0,0};
 scissor.extent = swapchain.extent;

 // Specify this as a dynamic state of a pipeline for modularity.
 // Very common and gives flexibility, where all implementations can handle this without a performance penalty.
 VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
 VkPipelineDynamicStateCreateInfo dynamic_state = {};
 dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
 dynamic_state.dynamicStateCount = sizeof(dynamic_states)/sizeof(dynamic_states[0]);
 dynamic_state.pDynamicStates = dynamic_states;

 // NOTICE: For dynamic states no actual pointers to data were defined below, this is going to be set up on draw time.
 VkPipelineViewportStateCreateInfo viewport_state = {};
 viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
 viewport_state.viewportCount = 1;
 viewport_state.scissorCount = 1;

 VkPipelineRasterizationStateCreateInfo rasterizer_info = {};
 rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
 rasterizer_info.depthClampEnable = VK_FALSE; // Fragments that are beyouind near and far planes are clamped to the view if VK_TRUE.
 rasterizer_info.rasterizerDiscardEnable = VK_FALSE; // Geometry never passes through the rasterizer stage if VK_TRUE.
 rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL; // How fragments are generated for geometry (GPU feature if using other than FILL!)
 rasterizer_info.lineWidth = 1.0f; // Thickness of lines in number of fragments. GPU feature for > 1.0;
 rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT; // Type of culling to use.
 rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE; //  Vertex irder fir faces??
 rasterizer_info.depthBiasEnable = VK_FALSE;      // Optional values below and here!!!
 rasterizer_info.depthBiasConstantFactor = 0.0f; // Altering of depth values by adding a constant value or biasing them based on a fragments slope.
 rasterizer_info.depthBiasClamp = 0.0f;           // Just disable for now
 rasterizer_info.depthBiasSlopeFactor = 0.0f;

 /**
  * Multisampling confuguration, one of th e ways to perform anti-aliasing.
  * Combines the fragment shader results of multiple polygons that rasterize to the same pixel.
  * Occurs mostly along edges.
  * Read more about this TODO
  */
 VkPipelineMultisampleStateCreateInfo multisample_info = {};
 multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
 multisample_info.sampleShadingEnable = VK_FALSE;
 multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
 multisample_info.minSampleShading = 1.0f;
 multisample_info.pSampleMask = NULL;
 multisample_info.alphaToCoverageEnable = VK_FALSE;
 multisample_info.alphaToOneEnable = VK_FALSE;

 // Configuration per attached framebuffer - VkPipelineColorBlendAttachmentState
 // Global config - VkPipelineColorBlendStateCreateInfo
 // Much to choose between, read on need.
 VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
 color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
 color_blend_attachment_state.blendEnable = VK_FALSE;
 // Many other settings, pseudocode for them on the web.
 
 VkPipelineColorBlendStateCreateInfo color_blend_info = {};
 color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
 color_blend_info.logicOpEnable = VK_FALSE;
 color_blend_info.logicOp = VK_LOGIC_OP_COPY;
 color_blend_info.attachmentCount = 1;
 color_blend_info.pAttachments = &color_blend_attachment_state;
 color_blend_info.blendConstants[0] = 0.0f; // Here we can specify constants for calculations in the attachments.

 // Pipeline layout
 // Specifying the uniform (global shader values) to pass in information like vertices, transformation matrices.
 // Even when empty it has to be a pipeline layout.
 VkPipelineLayoutCreateInfo pipeline_layout_info = {};
 pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

 VkResult res = vkCreatePipelineLayout(device, &pipeline_layout_info, NULL, &pipeline_layout);
 if(res != VK_SUCCESS) {
    printf("Failed to create pipeline layout! Code: %d\n", res);
    vkDestroyShaderModule(device, vertex_shader_module, NULL);
    vkDestroyShaderModule(device, fragment_shader_module, NULL);
    return FAILURE;
 }

 VkGraphicsPipelineCreateInfo pipeline_info = {};
 pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
 pipeline_info.stageCount = 2;
 pipeline_info.pStages = shader_stages;

 pipeline_info.pVertexInputState = &vertex_input_info;
 pipeline_info.pInputAssemblyState = &input_assembly_info;
 pipeline_info.pViewportState = &viewport_state;
 pipeline_info.pRasterizationState = &rasterizer_info;
 pipeline_info.pMultisampleState = &multisample_info;
 pipeline_info.pDepthStencilState = NULL;
 pipeline_info.pColorBlendState = &color_blend_info;
 pipeline_info.pDynamicState = &dynamic_state;

 pipeline_info.layout = pipeline_layout;
 pipeline_info.renderPass = render_pass;
 pipeline_info.subpass = 0; // Reference to the used uniform variable through the shader.
                            
 pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Derive from another pipeline by handle or index (VK_PIPELINE_CREATE_DERIVATIVE_BIT must be on!)
 pipeline_info.basePipelineIndex = -1;

 // This vk call is designed to produce multiple objects in a single call
 // Second parameter is a cache used to store relevant data for reuse over multiple calls and program executions (if stored into a file)
 res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &graphics_pipeline);
                                          
 if(res != VK_SUCCESS) {
    printf("Failed to create graphics pipeline. Cpde: %d\n", res);
    vkDestroyShaderModule(device, vertex_shader_module, NULL);
    vkDestroyShaderModule(device, fragment_shader_module, NULL);
    return FAILURE;
 }

 vkDestroyShaderModule(device, vertex_shader_module, NULL);
 vkDestroyShaderModule(device, fragment_shader_module, NULL);
 return SUCCESS;
}

int vk_create_framebuffers() {
 swapchain.p_framebuffers = GET_ARRAY(VkFramebuffer, swapchain.swapchain_image_count);

 for(int i = 0; i < swapchain.swapchain_image_count; i++) {
    VkImageView attachments[] = {swapchain.p_swapchain_image_views[i]};
    VkFramebufferCreateInfo framebuffer_info;
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass;
    framebuffer_info.attachmentCount = 1; // Specify the VkImageView that should be bound to the respective attachment in render pass.
    framebuffer_info.pAttachments = attachments;
    framebuffer_info.width = swapchain.extent.width;
    framebuffer_info.height = swapchain.extent.height;
    framebuffer_info.layers = 1; // As the image array layers

    VkResult res = vkCreateFramebuffer(device, &framebuffer_info, NULL, &swapchain.p_framebuffers[i]);
    if(res != VK_SUCCESS) {
      printf("Failed to create a framebuffer");
      return FAILURE;
    }
 }
 return SUCCESS;

}

int vk_create_command_pool() {
  struct vk_utils_info_s utils_info = {.physical_device = physical_device, .surface = surface};
  source_vk_utils(utils_info);

  struct QueueFamilyIndices indices = findQueueFamilies();

  VkCommandPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allow command buffers to be rerecorded individually (TRANSIENT_BIT) for command buffers to be rerecorded with new commands very often.
  pool_info.queueFamilyIndex = indices.graphics_family.value;

  VkResult res = vkCreateCommandPool(device, &pool_info, NULL, &command_pool);
  if(res != VK_SUCCESS) {
    printf("Failed to create a command pool! Code: %d\n", res);
    return FAILURE;
  }

  return SUCCESS;

}

int vk_create_command_buffer() {
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Submitted to a queue for execution, SECONDARY - not submitted directly but by a primary c.b.
  alloc_info.commandBufferCount = 1; // Allocating just one command buffer

  VkResult res = vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);
  if(res != VK_SUCCESS) {
    printf("Failed to allocate command buffers! Code: %d\n", res);
    return FAILURE;
  } 
  return SUCCESS;
}

static VkSemaphore image_available_sp;
static VkSemaphore render_finished_sp;
VkFence in_flight_fence;

int vk_create_sync_objects(){
  VkSemaphoreCreateInfo semaphore_info = {};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkFenceCreateInfo fence_info = {};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Holds fence active so it doesnt wait in draw_frame();

  if(vkCreateSemaphore(device, &semaphore_info, NULL, &image_available_sp) != VK_SUCCESS ||
     vkCreateSemaphore(device, &semaphore_info, NULL, &render_finished_sp) != VK_SUCCESS ||
     vkCreateFence(device, &fence_info, NULL, &in_flight_fence)) {
    printf("Failed to create sync objects!");
    return FAILURE;
  }
  return SUCCESS;
}


static void recordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index) {
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = 0; // opt How the recording shoud happen (more when)
  begin_info.pInheritanceInfo = NULL; // opt // Only relevant for secondary command buffers.

  VkResult res = vkBeginCommandBuffer(command_buffer, &begin_info); // Resetted recording once a call to begin happens again.
  if(res != VK_SUCCESS) {
    printf("Failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = render_pass;
  render_pass_info.framebuffer = swapchain.p_framebuffers[image_index];
  render_pass_info.renderArea.offset = (VkOffset2D){0, 0}; // Defines where shader loads and stores will take place
  render_pass_info.renderArea.extent = swapchain.extent;    /////

  VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  render_pass_info.clearValueCount = 1; // Clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR
  render_pass_info.pClearValues = &clear_color;

  // Record commands has this syntax below
  // All return void so error handeling after end of recording
  vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE); // Render pass commands wil be embedded in the primary command buffer, no secondary buffers will be executed.
  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline); // Second arg: What type of pipeline

  // These were dynamic, so we need to set them in the command buffer before draw. TODO Remove in earlier declaration??
  VkViewport viewport = {}; // Define the transformation from the image to the framebuffer.
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapchain.extent.width;
  viewport.height = (float)swapchain.extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(command_buffer, 0, 1, &viewport);

  VkRect2D scissor = {}; // Define where pixels will actually be stored (outside are disregarded by the rasterizer)
  scissor.offset = (VkOffset2D){0,0};
  scissor.extent = swapchain.extent;
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);

  // Params: Vertex count, instance count (if using instancing), Offset of gl_VertexIndex, Offset of gl_InstanceIndex
  vkCmdDraw(command_buffer,3, 1, 0, 0);

  vkCmdEndRenderPass(command_buffer);

  res = vkEndCommandBuffer(command_buffer);
  if(res != VK_SUCCESS) {
    printf("Failed to record command buffer!");
  }

}

static void draw_frame() {
 vkWaitForFences(device, 1, &in_flight_fence, VK_TRUE, UINT64_MAX);
 vkResetFences(device, 1, &in_flight_fence);

 uint32_t image_index;
 vkAcquireNextImageKHR(device, swapchain.swapchain, UINT64_MAX, image_available_sp, VK_NULL_HANDLE, &image_index);
 vkResetCommandBuffer(command_buffer, 0); // Make sure it is able to be recorded.

 recordCommandBuffer(command_buffer, image_index); // Record commands

 VkSubmitInfo submit_info = {};
 submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

 //NOTE: Each wait stage array entry corresponds to the dependency entry.
 VkSemaphore wait_dependencies[] = {image_available_sp};
 VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // In which stage to wait (graphics pipeline - writes to the color attachement)
 submit_info.waitSemaphoreCount = 1;
 submit_info.pWaitSemaphores = wait_dependencies;
 submit_info.pWaitDstStageMask = wait_stages;
 submit_info.commandBufferCount = 1;
 submit_info.pCommandBuffers = &command_buffer;

 VkSemaphore signal_sp[] = {render_finished_sp};
 submit_info.signalSemaphoreCount = 1;
 submit_info.pSignalSemaphores = signal_sp;

 VkResult res = vkQueueSubmit(queues.graphics_queue, 1, &submit_info, in_flight_fence);
 if(res != VK_SUCCESS) {
    printf("Failed to submit draw command buffer!");
 }

 VkPresentInfoKHR present_info = {};
 present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
 present_info.waitSemaphoreCount = 1;
 present_info.pWaitSemaphores = signal_sp;

 VkSwapchainKHR swapchains[] = {swapchain.swapchain};
 present_info.swapchainCount = 1;
 present_info.pSwapchains = swapchains;
 present_info.pImageIndices = &image_index;
 present_info.pResults = NULL; // Specify an array of VkResult for each swapchain if presentation was successfull, not necessary on only one.

 vkQueuePresentKHR(queues.present_queue, &present_info);

}

void vk_loop() {

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    draw_frame();
  }
  vkDeviceWaitIdle(device);
}

int vk_cleanup() {
  printf("Terminating...\n");

  // DESTRUCTION PRE L-DEVICE //
  vkDestroySemaphore(device, image_available_sp, NULL);
  vkDestroySemaphore(device, render_finished_sp, NULL);
  vkDestroyFence(device, in_flight_fence, NULL);
  vkDestroyCommandPool(device, command_pool, NULL);
  for(int i = 0; i < swapchain.swapchain_image_count; i++) {
    vkDestroyFramebuffer(device, swapchain.p_framebuffers[i], NULL);
  }
  vkDestroyPipeline(device, graphics_pipeline, NULL);
  vkDestroyPipelineLayout(device, pipeline_layout, NULL);
  vkDestroyRenderPass(device, render_pass, NULL);
  vkDestroySwapchainKHR(device, swapchain.swapchain, NULL);
  for(int i = 0; i < swapchain.swapchain_image_count; i++) {
    vkDestroyImageView(device, swapchain.p_swapchain_image_views[i], NULL);
  }
  free(swapchain.p_swapchain_image_views);
  free(swapchain.p_swapchain_images);
  free(swapchain.p_framebuffers);
  /////////////////////////////

  vkDestroyDevice(device, NULL);
  vkDestroySurfaceKHR(instance, surface, NULL);
  vkDestroyInstance(instance, NULL);
  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
