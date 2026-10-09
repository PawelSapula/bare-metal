#include "buffers.h"
#include "vk_instance.h"
#include <vk_utils.h>

static int impl_create_framebuffers();
static int impl_create_command_buffer();
static int impl_create_command_pool();

struct buffer_api buffer_api = {
  .create_frambuffers = impl_create_framebuffers,
  .create_command_buffer = impl_create_command_buffer,
  .create_command_pool = impl_create_command_pool
};

static void __attribute__((constructor)) init_api() {
  api.buffer_api = buffer_api;
}

static int impl_create_framebuffers() {
 vk_inst.graphics.swapchain_s.p_framebuffers = GET_ARRAY(VkFramebuffer, vk_inst.graphics.swapchain_s.swapchain_image_count);

 for(int i = 0; i < vk_inst.graphics.swapchain_s.swapchain_image_count; i++) {
    VkImageView attachments[] = {vk_inst.graphics.swapchain_s.p_swapchain_image_views[i]};
    VkFramebufferCreateInfo framebuffer_info;
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = vk_inst.graphics.render_pass;
    framebuffer_info.attachmentCount = 1; // Specify the VkImageView that should be bound to the respective attachment in render pass.
    framebuffer_info.pAttachments = attachments;
    framebuffer_info.width = vk_inst.graphics.swapchain_s.extent.width;
    framebuffer_info.height = vk_inst.graphics.swapchain_s.extent.height;
    framebuffer_info.layers = 1; // As the image array layers

    VkResult res = vkCreateFramebuffer(vk_inst.devices.device, &framebuffer_info, NULL, &vk_inst.graphics.swapchain_s.p_framebuffers[i]);
    if(res != VK_SUCCESS) {
      printf("Failed to create a framebuffer");
      return FAILURE;
    }
 }
 return SUCCESS;

}

static int impl_create_command_buffer() {
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = vk_inst.graphics.command_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Submitted to a queue for execution, SECONDARY - not submitted directly but by a primary c.b.
  alloc_info.commandBufferCount = 1; // Allocating just one command buffer

  VkResult res = vkAllocateCommandBuffers(vk_inst.devices.device, &alloc_info, &vk_inst.graphics.command_buffer);
  if(res != VK_SUCCESS) {
    printf("Failed to allocate command buffers! Code: %d\n", res);
    return FAILURE;
  } 
  return SUCCESS;
}

static int impl_create_command_pool() {
  struct vk_utils_info_s utils_info = {.physical_device = vk_inst.devices.physical_device, .surface = vk_inst.graphics.swapchain_s.surface};
  source_vk_utils(utils_info);

  VkCommandPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allow command buffers to be rerecorded individually (TRANSIENT_BIT) for command buffers to be rerecorded with new commands very often.
  pool_info.queueFamilyIndex = vk_inst.devices.queue_families.graphics_family.value;

  VkResult res = vkCreateCommandPool(vk_inst.devices.device, &pool_info, NULL, &vk_inst.graphics.command_pool);
  if(res != VK_SUCCESS) {
    printf("Failed to create a command pool! Code: %d\n", res);
    return FAILURE;
  }

  return SUCCESS;

}
