#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosition;
layout (location = 2) in vec3 fragNormal;

layout (location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform GlobalUbo {
  mat4 projectionMatrix;
  mat4 viewMatrix;
  vec4 ambientColor;
  vec4 lightPosition;
  vec4 lightColor;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

void main() {
  vec3 directionToLight = ubo.lightPosition.xyz - fragPosition;
  float attenuation =  3.0 / dot(directionToLight, directionToLight);
  vec3 ambientLight = ubo.ambientColor.xyz * ubo.ambientColor.w;
  vec3 pointLight = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;

  float diffuseIntensity = max(dot(normalize(fragNormal), normalize(directionToLight)), 0);
  vec3 diffuseLight = pointLight * diffuseIntensity;

  outColor = vec4(((diffuseLight + ambientLight) * fragColor), 1.0);
}
