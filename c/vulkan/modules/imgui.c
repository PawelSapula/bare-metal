#include "imgui.h"
#include "instance.h"
#include "utils.h"
#include <dcimgui.h>
#include <dcimgui_impl_glfw.h>
#include <dcimgui_impl_vulkan.h>
#include <stdio.h>

static void impl_init_gui();
static void impl_shutdown_gui();
static void impl_draw();
static void impl_build_frame();
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

struct imgui_api gui_api = {
  .init_gui = impl_init_gui,
  .shoutdown_gui = impl_shutdown_gui,
  .draw = impl_draw,
  .build_frame = impl_build_frame,
  .vars = {0}
};

static void __attribute__((constructor)) init_api() {
  api.gui_api = gui_api; 
}

static void impl_init_gui() {
    ImGui_CreateContext(NULL);
    ImGuiIO* io = ImGui_GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->DisplaySize.x = 100;
    io->DisplaySize.y = 100;

    glfwSetScrollCallback(vk_inst.instance.window, scroll_callback); // Testing
                                                                 //
    cImGui_ImplGlfw_InitForVulkan(vk_inst.instance.window, true); 
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vk_inst.instance.vk_instance;
    init_info.PhysicalDevice = vk_inst.devices.physical_device;
    init_info.Device = vk_inst.devices.device;
    init_info.QueueFamily = vk_inst.graphics.swapchain_s.queues.graphics_family_index.value;
    init_info.Queue = vk_inst.graphics.swapchain_s.queues.graphics_queue;
    init_info.MinImageCount = vk_inst.graphics.swapchain_s.swapchain_image_count;
    init_info.ImageCount = vk_inst.graphics.swapchain_s.swapchain_image_count;
    init_info.PipelineInfoMain.RenderPass = vk_inst.graphics.render_pass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.DescriptorPoolSize = 8;

    cImGui_ImplVulkan_Init(&init_info);
}

static void impl_shutdown_gui() {
  cImGui_ImplVulkan_Shutdown();
  cImGui_ImplGlfw_Shutdown();
  ImGui_DestroyContext(ImGui_GetCurrentContext());
}


static void impl_build_frame() {
  cImGui_ImplGlfw_NewFrame();
    cImGui_ImplVulkan_NewFrame();
    ImGui_NewFrame();
    ImGui_Begin("Hello World!", NULL, 0);
    ImGui_Text("Scroll X: %f Y: %f", api.gui_api.vars.scroll_x, api.gui_api.vars.scroll_y);
    ImGui_Text("Cursor X: %f Y: %f", api.gui_api.vars.cursor_x, api.gui_api.vars.cursor_y);
    ImGui_End();
    ImGui_Render();

}

static void impl_draw() { 
  cImGui_ImplVulkan_RenderDrawData(ImGui_GetDrawData(), vk_inst.graphics.command_buffer);
}


static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { // Only y-offset on a mouse, x for mousepads etc.
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos); // From top left of window content area
  api.gui_api.vars.cursor_x = xpos;
  api.gui_api.vars.cursor_y = ypos;

  api.gui_api.vars.scroll_y += yoffset;
  api.gui_api.vars.scroll_y = CLAMP(api.gui_api.vars.scroll_y, -10, 10);
}
