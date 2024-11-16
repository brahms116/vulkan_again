#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosition;
layout (location = 2) in vec3 fragNormal;
layout (location = 3) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

struct PointLight {
  vec4 position;
  vec4 color; // w is intensity
};

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

layout(set=0, binding=1) uniform sampler2D shadowMapSampler;

layout(set=1, binding=0) uniform sampler2D texSampler;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

void main() {
  vec3 surfaceNormal = normalize(fragNormal);

  //  Variable storing the summation result, with the initial value being the ambient light
  vec3 diffuseLight = ubo.ambientColor.xyz * ubo.ambientColor.w;

  //  Variable storing the summation result of specular lighting
  vec3 specularLight = vec3(0.0);

  vec3 cameraPositionWorld = ubo.inverseViewMatrix[3].xyz;
  vec3 normalizedViewDirection = normalize(cameraPositionWorld - fragPosition);

  for (int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 directionToLight = light.position.xyz - fragPosition;
    vec3 normalizedDirectionToLight = normalize(directionToLight);
    float attenuation  = 1.0 / dot(directionToLight, directionToLight);

    float cosAngIncidence = max(dot(surfaceNormal, normalizedDirectionToLight), 0);

    float netDiffuseIntensityFactor = light.color.w * cosAngIncidence * attenuation;
    diffuseLight += light.color.xyz * netDiffuseIntensityFactor;

    // Specular light calculation
    vec3 halfVector = normalize(normalizedDirectionToLight + normalizedViewDirection);
    float blinnTerm = clamp(dot(surfaceNormal, halfVector), 0, 1);
    blinnTerm = pow(blinnTerm, 512.0);

    float netSpecularIntensityFactor = light.color.w * attenuation * blinnTerm;
    specularLight += light.color.xyz * netSpecularIntensityFactor;
  }

  vec4 netLight = vec4((specularLight + diffuseLight), 1.0);
  outColor = netLight * texture(shadowMapSampler, fragTexCoord); 
}
