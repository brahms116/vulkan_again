#include "va_renderer.hpp"

#include <array>
#include <cassert>
#include <stdexcept>

namespace va {

VaRenderer::VaRenderer(VaWindow &window, VaDevice &device)
    : vaWindow{window}, vaDevice{device} {
  recreateSwapChain();
  createCommandBuffers();
}

VaRenderer::~VaRenderer() { freeCommandBuffers(); }

void VaRenderer::freeCommandBuffers() {
  vkFreeCommandBuffers(vaDevice.device(), vaDevice.getCommandPool(),
                       static_cast<uint32_t>(commandBuffers.size()),
                       commandBuffers.data());
  commandBuffers.clear();
}

void VaRenderer::recreateSwapChain() {
  auto extent = vaWindow.getExtent();

  while (extent.width == 0 || extent.height == 0) {
    extent = vaWindow.getExtent();
    glfwPollEvents();
  }

  vkDeviceWaitIdle(vaDevice.device());
  if (vaSwapChain == nullptr) {
    vaSwapChain = std::make_unique<VaSwapChain>(vaDevice, extent);
  } else {
    std::shared_ptr<VaSwapChain> oldSwapChain = std::move(vaSwapChain);
    vaSwapChain = std::make_unique<VaSwapChain>(vaDevice, extent, oldSwapChain);

    if (!oldSwapChain->compareSwapFormats(*vaSwapChain)) {
      throw std::runtime_error("SwapChain formats incompatible");
    }
  }
  /* createPipeline(); */
}

void VaRenderer::createCommandBuffers() {
  commandBuffers.resize(VaSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = vaDevice.getCommandPool(),
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
  };

  if (vkAllocateCommandBuffers(vaDevice.device(), &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

VkCommandBuffer VaRenderer::beginFrame() {
  assert(!isFrameStarted &&
         "Cannot call begin Frame while already in progress");

  VkResult result = vaSwapChain->acquireNextImage(&currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("Failed to acquire next image");
  }

  isFrameStarted = true;

  auto commandBuffer = getCurrentCommandBuffer();

  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  };

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  return commandBuffer;
}

void VaRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted &&
         "Cannot call begin render pass while frame is not in progress");

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};

  VkRenderPassBeginInfo renderPassInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = vaSwapChain->getRenderPass(),
      .framebuffer = vaSwapChain->getFrameBuffer(currentImageIndex),
      .renderArea = {.offset = {0, 0},
                     .extent = vaSwapChain->getSwapChainExtent()},
      .clearValueCount = static_cast<uint32_t>(clearValues.size()),
      .pClearValues = clearValues.data(),
  };

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(vaSwapChain->getSwapChainExtent().width);
  viewport.height =
      static_cast<float>(vaSwapChain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scisscor{{0, 0}, vaSwapChain->getSwapChainExtent()};

  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scisscor);
}

void VaRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted &&
         "Cannot call end render pass while frame is not in progress");
  vkCmdEndRenderPass(commandBuffer);
}

void VaRenderer::endFrame() {
  assert(isFrameStarted &&
         "Cannot call end frame while frame is not in progress");
  auto commandBuffer = getCurrentCommandBuffer();
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
  auto result =
      vaSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      vaWindow.wasWindowResized()) {
    vaWindow.resetWindowResizedFlag();
    recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to submit command buffer");
  }
  isFrameStarted = false;
  currentFrameIndex = (currentFrameIndex + 1) % VaSwapChain::MAX_FRAMES_IN_FLIGHT;
}

} // namespace va
