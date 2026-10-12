#ifndef IMGUI_H
#define IMGUI_H

#include <dcimgui.h>

struct imgui_variables {
  double scroll_x, scroll_y;
  double cursor_x, cursor_y;
};

struct imgui_api {
  struct imgui_variables vars;
  void (*init_gui)();
  void (*shoutdown_gui)();
  void (*build_frame)();
  void (*draw)();
};

#endif /* IMGUI_H */
