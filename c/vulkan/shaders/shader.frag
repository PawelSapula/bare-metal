#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor; // Specify out variable with index of the framebuffer.


void main() {
  outColor = vec4(fragColor, 1.0);
}
