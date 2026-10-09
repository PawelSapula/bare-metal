#ifndef PIPELINE_H
#define PIPELINE_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct pipeline_api {
  int (*create_render_pass)();
  int (*create_graphics_pipeline)();
};

#endif /* PIPELINE_H */
