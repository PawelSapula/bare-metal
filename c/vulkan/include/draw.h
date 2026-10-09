#ifndef DRAW_H
#define DRAW_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct draw_api {
  int (*create_sync_objects)();
  void (*destroy_sync_objects)();
  void (*draw_frame)();
};

#endif /* DRAW_H */
