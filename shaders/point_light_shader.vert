#version 450

struct PointLight {
  vec4 position;
  vec4 color; // w is intensity
};

layout(set=0, binding=0) uniform GlobalUbo {
  mat4 projectionMatrix;
  mat4 viewMatrix;
  vec4 ambientColor;
  PointLight pointLights[10];
  int numLights;
} ubo;

layout(push_constant) uniform Push {
  vec4 position;
  vec4 color;
  float radius;
} push;

layout (location = 0) out vec2 fragOffset;

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

void main() {
  fragOffset = OFFSETS[gl_VertexIndex];

  vec4 lightPositionInCameraSpace = ubo.viewMatrix * push.position;
  vec4 cameraSpace = lightPositionInCameraSpace + push.radius * vec4(fragOffset, 0.0, 0.0);
  gl_Position = ubo.projectionMatrix * cameraSpace;
}
