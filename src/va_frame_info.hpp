#pragma once

#include "va_camera.hpp"

// lib
#include <vulkan/vulkan.h>

namespace va {
struct FrameInfo {
  int frameIndex;
  float frameTime;
  VkCommandBuffer commandBuffer;
  VaCamera &camera;
};
} // namespace va
