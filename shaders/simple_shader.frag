#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosition;
layout (location = 2) in vec3 fragNormal;

layout (location = 0) out vec4 outColor;

struct PointLight {
  vec4 position;
  vec4 color; // w is intensity
};

layout(set=0, binding=0) uniform GlobalUbo {
  mat4 projectionMatrix;
  mat4 viewMatrix;
  mat4 inverseViewMatrix;
  vec4 ambientColor;
  PointLight pointLights[10];
  int numLights;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

void main() {
  vec3 surfaceNormal = normalize(fragNormal);

  //  Variable storing the summation result, with the initial value being the ambient light
  vec3 diffuseLight = ubo.ambientColor.xyz * ubo.ambientColor.w;

  for (int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 directionToLight = light.position.xyz - fragPosition;

    float diffuseLightFactor = max(dot(surfaceNormal, normalize(directionToLight)), 0);
    float attenuation  = 1.0 / dot(directionToLight, directionToLight);

    float netIntensityFactor = light.color.w * diffuseLightFactor * attenuation;

    diffuseLight += light.color.xyz * netIntensityFactor;
    // diffuseLight += light.color.xyz;
  }

  outColor = vec4((diffuseLight * fragColor), 1.0);
}
