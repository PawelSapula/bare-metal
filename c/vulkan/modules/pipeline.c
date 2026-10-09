#include <stdio.h>

#include "pipeline.h"
#include <vk_instance.h>
#include <vk_shader_utils.h>


static int impl_create_render_pass();
static int impl_create_graphics_pipeline();

struct pipeline_api pipeline_api = {
  .create_graphics_pipeline = impl_create_graphics_pipeline,
  .create_render_pass = impl_create_render_pass
};

static void __attribute__((constructor)) init_api() {
  api.pipeline_api = pipeline_api;
}

static int impl_create_render_pass() {
 VkAttachmentDescription color_attachment = {}; 
 color_attachment.format = vk_inst.graphics.swapchain_s.image_format;
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

 VkResult res = vkCreateRenderPass(vk_inst.devices.device, &render_pass_info, NULL, &vk_inst.graphics.render_pass);
 if(res != VK_SUCCESS) {
    printf("Failed to create render pass! Code: %d\n", res);
    return FAILURE;
 }
 return SUCCESS;

}


static int impl_create_graphics_pipeline() {
 source_shader_utils((struct vk_utils_info_s){.device = vk_inst.devices.device});
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

 /*
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
 */

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

 VkResult res = vkCreatePipelineLayout(vk_inst.devices.device, &pipeline_layout_info, NULL, &vk_inst.graphics.pipeline_layout);
 if(res != VK_SUCCESS) {
    printf("Failed to create pipeline layout! Code: %d\n", res);
    vkDestroyShaderModule(vk_inst.devices.device, vertex_shader_module, NULL);
    vkDestroyShaderModule(vk_inst.devices.device, fragment_shader_module, NULL);
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

   pipeline_info.layout = vk_inst.graphics.pipeline_layout;
 pipeline_info.renderPass = vk_inst.graphics.render_pass;
 pipeline_info.subpass = 0; // Reference to the used uniform variable through the shader.
                            
 pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Derive from another pipeline by handle or index (VK_PIPELINE_CREATE_DERIVATIVE_BIT must be on!)
 pipeline_info.basePipelineIndex = -1;

 // This vk call is designed to produce multiple objects in a single call
 // Second parameter is a cache used to store relevant data for reuse over multiple calls and program executions (if stored into a file)
 res = vkCreateGraphicsPipelines(vk_inst.devices.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &vk_inst.graphics.graphics_pipeline);
                                          
 if(res != VK_SUCCESS) {
    printf("Failed to create graphics pipeline. Cpde: %d\n", res);
    vkDestroyShaderModule(vk_inst.devices.device, vertex_shader_module, NULL);
    vkDestroyShaderModule(vk_inst.devices.device, fragment_shader_module, NULL);
    return FAILURE;
 }

 vkDestroyShaderModule(vk_inst.devices.device, vertex_shader_module, NULL);
 vkDestroyShaderModule(vk_inst.devices.device, fragment_shader_module, NULL);
 return SUCCESS;

};

