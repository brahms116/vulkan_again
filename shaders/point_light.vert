#version 450

layout(set=0, binding=0) uniform GlobalUbo {
  mat4 projectionMatrix;
  mat4 viewMatrix;
  vec4 ambientColor;
  vec4 lightPosition;
  vec4 lightColor;
} ubo;

layout (location = 0) out vec2 fragOffset;

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

void main() {}
