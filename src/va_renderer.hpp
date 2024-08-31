#pragma once

#include "va_engine_device.hpp"
#include "va_swap_chain.hpp"
#include "va_window.hpp"
#include <cassert>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>

namespace va {

class VaRenderer {
public:
  VaRenderer(VaWindow &vaWindow, VaDevice &vaDevice);
  ~VaRenderer();

  VaRenderer(const VaRenderer &) = delete;
  VaRenderer &operator=(const VaRenderer &) = delete;

  bool isFrameInProgress() const { return isFrameStarted; }

  VkCommandBuffer getCurrentCommandBuffer() const {
    assert(isFrameStarted &&
           "Cannot get command buffer when frame not in progress");
    return commandBuffers[currentFrameIndex];
  }

  VkRenderPass getSwapChainRenderPass() const {
    return vaSwapChain->getRenderPass();
  }

  float getAspectRatio() const { return vaSwapChain->extentAspectRatio(); }

  VkCommandBuffer beginFrame();
  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endFrame();

  int getFrameIndex() const {
    assert(isFrameStarted &&
           "Cannot get current frame index when frame not in progress");
    return currentFrameIndex;
  }

private:
  void createCommandBuffers();
  void recreateSwapChain();
  void freeCommandBuffers();

  VaWindow &vaWindow;
  VaDevice &vaDevice;
  uint32_t currentImageIndex{0};
  bool isFrameStarted{false};
  int currentFrameIndex{0};

  std::unique_ptr<VaSwapChain> vaSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;
};
} // namespace va
