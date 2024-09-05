#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 frag_color;

layout(push_constant) uniform Push {
  mat4 transform;
  mat4 normalMatrix;
} push;

void main() {
  gl_Position = push.transform * vec4(position, 1.0);

  vec3 DIR_TO_LIGHT = normalize(vec3(1, -3, -1));
  float AMBIENT = 0.2;

  mat3 inverseTranspose = transpose(inverse(mat3(push.model)));

  vec3 normalWorldSpace = normalise(mat3(push.normalMatrix) * normal);

  float lightIntensity = min(AMBIENT + max(dot(normalWorldSpace, DIR_TO_LIGHT), 0) ,1.0);

  frag_color = lightIntensity *  color;
}


