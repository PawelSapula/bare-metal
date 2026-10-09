#include "draw.h"
#include "vk_instance.h"

static int  impl_create_sync_objects();
static void impl_draw_frame();
static void impl_destroy_sync_objects();

struct draw_api draw_api = {
  .draw_frame = impl_draw_frame,
  .create_sync_objects = impl_create_sync_objects,
  .destroy_sync_objects = impl_destroy_sync_objects
};

static void __attribute__((constructor)) init_api() {
 api.draw_api = draw_api;  
}

static VkSemaphore image_available_sp;
static VkSemaphore render_finished_sp;
static VkFence in_flight_fence;

static int impl_create_sync_objects(){
  VkSemaphoreCreateInfo semaphore_info = {};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkFenceCreateInfo fence_info = {};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Holds fence active so it doesnt wait in draw_frame();

  if(vkCreateSemaphore(vk_inst.devices.device, &semaphore_info, NULL, &image_available_sp) != VK_SUCCESS ||
     vkCreateSemaphore(vk_inst.devices.device, &semaphore_info, NULL, &render_finished_sp) != VK_SUCCESS ||
     vkCreateFence(vk_inst.devices.device, &fence_info, NULL, &in_flight_fence)) {
    printf("Failed to create sync objects!");
    return FAILURE;
  }
  return SUCCESS;
}

static void impl_destroy_sync_objects() {
  vkDestroySemaphore(vk_inst.devices.device, image_available_sp, NULL);
  vkDestroySemaphore(vk_inst.devices.device, render_finished_sp, NULL);
  vkDestroyFence(vk_inst.devices.device, in_flight_fence, NULL);
};


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
  render_pass_info.renderPass = vk_inst.graphics.render_pass;
  render_pass_info.framebuffer = vk_inst.graphics.swapchain_s.p_framebuffers[image_index];
  render_pass_info.renderArea.offset = (VkOffset2D){0, 0}; // Defines where shader loads and stores will take place
  render_pass_info.renderArea.extent = vk_inst.graphics.swapchain_s.extent;    /////

  VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  render_pass_info.clearValueCount = 1; // Clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR
  render_pass_info.pClearValues = &clear_color;

  // Record commands has this syntax below
  // All return void so error handeling after end of recording
  vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE); // Render pass commands wil be embedded in the primary command buffer, no secondary buffers will be executed.
  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_inst.graphics.graphics_pipeline); // Second arg: What type of pipeline

  // These were dynamic, so we need to set them in the command buffer before draw. TODO Remove in earlier declaration??
  VkViewport viewport = {}; // Define the transformation from the image to the framebuffer.
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)vk_inst.graphics.swapchain_s.extent.width;
  viewport.height = (float)vk_inst.graphics.swapchain_s.extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(command_buffer, 0, 1, &viewport);

  VkRect2D scissor = {}; // Define where pixels will actually be stored (outside are disregarded by the rasterizer)
  scissor.offset = (VkOffset2D){0,0};
  scissor.extent = vk_inst.graphics.swapchain_s.extent;
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);

  // Params: Vertex count, instance count (if using instancing), Offset of gl_VertexIndex, Offset of gl_InstanceIndex
  vkCmdDraw(command_buffer,3, 1, 0, 0);

  vkCmdEndRenderPass(command_buffer);

  res = vkEndCommandBuffer(command_buffer);
  if(res != VK_SUCCESS) {
    printf("Failed to record command buffer!");
  }

}

static void impl_draw_frame() {
 vkWaitForFences(vk_inst.devices.device, 1, &in_flight_fence, VK_TRUE, UINT64_MAX);
 vkResetFences(vk_inst.devices.device, 1, &in_flight_fence);

 uint32_t image_index;
 vkAcquireNextImageKHR(vk_inst.devices.device, vk_inst.graphics.swapchain_s.swapchain, UINT64_MAX, image_available_sp, VK_NULL_HANDLE, &image_index);
 vkResetCommandBuffer(vk_inst.graphics.command_buffer, 0); // Make sure it is able to be recorded.

 recordCommandBuffer(vk_inst.graphics.command_buffer, image_index); // Record commands

 VkSubmitInfo submit_info = {};
 submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

 //NOTE: Each wait stage array entry corresponds to the dependency entry.
 VkSemaphore wait_dependencies[] = {image_available_sp};
 VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // In which stage to wait (graphics pipeline - writes to the color attachement)
 submit_info.waitSemaphoreCount = 1;
 submit_info.pWaitSemaphores = wait_dependencies;
 submit_info.pWaitDstStageMask = wait_stages;
 submit_info.commandBufferCount = 1;
 submit_info.pCommandBuffers = &vk_inst.graphics.command_buffer;

 VkSemaphore signal_sp[] = {render_finished_sp};
 submit_info.signalSemaphoreCount = 1;
 submit_info.pSignalSemaphores = signal_sp;

 VkResult res = vkQueueSubmit(vk_inst.graphics.swapchain_s.queues.graphics_queue, 1, &submit_info, in_flight_fence);
 if(res != VK_SUCCESS) {
    printf("Failed to submit draw command buffer!");
 }

 VkPresentInfoKHR present_info = {};
 present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
 present_info.waitSemaphoreCount = 1;
 present_info.pWaitSemaphores = signal_sp;

 VkSwapchainKHR swapchains[] = {vk_inst.graphics.swapchain_s.swapchain};
 present_info.swapchainCount = 1;
 present_info.pSwapchains = swapchains;
 present_info.pImageIndices = &image_index;
 present_info.pResults = NULL; // Specify an array of VkResult for each swapchain if presentation was successfull, not necessary on only one.

 vkQueuePresentKHR(vk_inst.graphics.swapchain_s.queues.present_queue, &present_info);

}

