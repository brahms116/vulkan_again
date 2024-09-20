#version 450

layout(set=0, binding=0) uniform GlobalUbo {
  mat4 projectionMatrix;
  mat4 viewMatrix;
  vec4 ambientColor;
  vec4 lightPosition;
  vec4 lightColor;
} ubo;

layout (location = 0) in vec2 fragOffset;

layout (location = 0) out vec4 outColor;

void main() {
  float distance = sqrt(dot(fragOffset,fragOffset));
  if (distance >= 1.0) {
    discard;
  }
  outColor = vec4(ubo.lightColor.xyz, 1.0);
}
