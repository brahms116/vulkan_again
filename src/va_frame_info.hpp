#pragma once

#include "va_camera.hpp"
#include "va_game_object.hpp"

// lib
#include <vulkan/vulkan.h>

namespace va {

struct GlobalUbo {
  glm::mat4 projectionMatrix;
  glm::mat4 viewMatrix;
  glm::vec4 ambientColor{1.f, 1.f, 1.f, .15f};
  glm::vec4 lightPosition{-1.f, -1.f, -1.f, 1.f};
  glm::vec4 lightColor{1.f};
};

struct FrameInfo {
  int frameIndex;
  float frameTime;
  VkCommandBuffer commandBuffer;
  VaCamera &camera;
  VkDescriptorSet descriptorSet;
  VaGameObject::Map &gameObjects;
};
} // namespace va
