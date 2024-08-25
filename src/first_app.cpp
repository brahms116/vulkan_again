#include "first_app.hpp"

#include <stdexcept>

#include <array>

#include <iostream>

namespace va {

FirstApp::FirstApp() {
  loadGameObjects();
  createPipelineLayout();
  recreateSwapChain();
  createCommandBuffers();
}

FirstApp::~FirstApp() {
  vkDestroyPipelineLayout(vaDevice.device(), pipelineLayout, nullptr);
}

void FirstApp::drawFrame() {
  uint32_t imageIndex;
  VkResult result = vaSwapChain->acquireNextImage(&imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("Failed to acquire next image");
  }

  recordCommandBuffer(imageIndex);
  result = vaSwapChain->submitCommandBuffers(&commandBuffers[imageIndex],
                                             &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      vaWindow.wasWindowResized()) {
    vaWindow.resetWindowResizedFlag();

    recreateSwapChain();
    return;
  }
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to submit command buffer");
  }
}

void FirstApp::loadGameObjects() {
  std::vector<VaModel::Vertex> vertices{
      {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
  };

  auto model = std::make_shared<VaModel>(vaDevice, vertices);
  auto triangle = VaGameObject::create();
  triangle.model = model;
  triangle.color = {1.f, 1.f, 1.f};
  triangle.transform2d.translation.x = 1.0f;
  triangle.transform2d.scale = {2.f, .5f};
  triangle.transform2d.rotation = 0.25 * glm::two_pi<float>();

  gameObjects.push_back(std::move(triangle));
}

void FirstApp::freeCommandBuffers() {
  vkFreeCommandBuffers(vaDevice.device(), vaDevice.getCommandPool(),
                       static_cast<uint32_t>(commandBuffers.size()),
                       commandBuffers.data());
  commandBuffers.clear();
}

void FirstApp::recreateSwapChain() {
  auto extent = vaWindow.getExtent();

  while (extent.width == 0 || extent.height == 0) {
    extent = vaWindow.getExtent();
    glfwPollEvents();
  }

  vkDeviceWaitIdle(vaDevice.device());
  if (vaSwapChain == nullptr) {
    vaSwapChain = std::make_unique<VaSwapChain>(vaDevice, extent);
  } else {
    vaSwapChain =
        std::make_unique<VaSwapChain>(vaDevice, extent, std::move(vaSwapChain));
    if (vaSwapChain->imageCount() != commandBuffers.size()) {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }
  createPipeline();
}

void FirstApp::recordCommandBuffer(int imageIndex) {
  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  };

  if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};

  VkRenderPassBeginInfo renderPassInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = vaSwapChain->getRenderPass(),
      .framebuffer = vaSwapChain->getFrameBuffer(imageIndex),
      .renderArea = {.offset = {0, 0},
                     .extent = vaSwapChain->getSwapChainExtent()},
      .clearValueCount = static_cast<uint32_t>(clearValues.size()),
      .pClearValues = clearValues.data(),
  };

  vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo,
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

  vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
  vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scisscor);

  renderGameObjects(commandBuffers[imageIndex]);

  vkCmdEndRenderPass(commandBuffers[imageIndex]);
  if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}

void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer) {
  vaPipeline->bindCommandBuffer(commandBuffer);
  for (auto &object : gameObjects) {

    object.transform2d.rotation =
        glm::mod(object.transform2d.rotation + 0.01f, glm::two_pi<float>());

    SimplePushConstantData push{};
    push.offset = object.transform2d.translation;
    push.color = object.color;
    push.transform = object.transform2d.mat2();

    std::cout << push.color[0] << " " << object.color[1] << " " << push.color[2]
              << std::endl;

    vkCmdPushConstants(commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT |
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(SimplePushConstantData), &push);

    object.model->bindCommandBuffer(commandBuffer);
    object.model->draw(commandBuffer);
  }
}
void FirstApp::createCommandBuffers() {
  commandBuffers.resize(vaSwapChain->imageCount());

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

void FirstApp::run() {
  while (!vaWindow.shouldClose()) {
    glfwPollEvents();
    drawFrame();

    vkDeviceWaitIdle(vaDevice.device());
  }
}

void FirstApp::createPipelineLayout() {
  VkPushConstantRange push{};
  push.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push.offset = 0;
  push.size = sizeof(SimplePushConstantData);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pSetLayouts = nullptr,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push};

  if (vkCreatePipelineLayout(vaDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Could not create pipeline layout");
  }
};

void FirstApp::createPipeline() {
  PipelineConfigInfo configInfo{};
  VaPipeline::setDefaultPipelineConfigInfo(configInfo);
  configInfo.renderPass = vaSwapChain->getRenderPass();
  configInfo.pipelineLayout = pipelineLayout;
  vaPipeline = std::make_unique<VaPipeline>(
      vaDevice, "./shaders/simple_shader.vert.spv",
      "./shaders/simple_shader.frag.spv", configInfo);
};

} // namespace va
