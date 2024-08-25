#version 450

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
  mat2 transform;
  vec4 color;
  vec2 offset;
} push;

void main() {
  outColor = push.color;
}
