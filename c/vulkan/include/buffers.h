#ifndef BUFFERS_H
#define BUFFERS_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>

struct buffer_api {
  int (*create_frambuffers)();
  int (*create_command_buffer)();
  int (*create_command_pool)();
};

#endif /*BUFFERS_H*/
