#version 450

struct PointLight {
  vec4 position;
  vec4 color; // w is intensity
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(set=0, binding=0) uniform GlobalUbo {
  mat4 projectionMatrix;
  mat4 viewMatrix;
  mat4 lightProjectionMatrix;
  mat4 lightViewMatrix;
  mat4 inverseViewMatrix;
  vec4 ambientColor;
  PointLight pointLights[10];
  int numLights;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
} push;

void main() {
  vec4 positionWorldSpace = push.modelMatrix * vec4(position, 1.0);
  gl_Position = ubo.lightProjectionMatrix *  (ubo.lightViewMatrix * positionWorldSpace);
}
