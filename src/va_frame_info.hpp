#pragma once

#include "va_camera.hpp"
#include "va_descriptors.hpp"
#include "va_game_object.hpp"

// lib
#include <vulkan/vulkan.h>

namespace va {

#define MAX_NUM_LIGHTS 10

struct PointLight {
  glm::vec4 position{};
  glm::vec4 color{};
};

struct GlobalUbo {
  glm::mat4 projectionMatrix{1.f};
  glm::mat4 viewMatrix{1.f};
  glm::mat4 inverseViewMatrix{1.f};
  glm::vec4 ambientColor{1.f, 1.f, 1.f, .15f};
  PointLight pointLights[MAX_NUM_LIGHTS];
  int numLights;
};

struct FrameInfo {
  int frameIndex;
  float frameTime;
  VkCommandBuffer commandBuffer;
  VaCamera &camera;
  VkDescriptorSet descriptorSet;
  VkDescriptorSet textureDescriptorSet;
  VaDescriptorWriter writer;
  VaGameObject::Map &gameObjects;
};
} // namespace va
