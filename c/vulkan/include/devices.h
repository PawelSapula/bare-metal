#ifndef DEVICES_H
#define DEVICES_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct device_api {
  int (*pick_physical_device)();
  int (*create_logical_device)();
};

#endif /* DEVICES_H */
