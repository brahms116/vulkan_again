#include "first_app.hpp"

#include <stdexcept>

namespace va {

FirstApp::FirstApp() {
  createPipelineLayout();
  createPipeline();
  createCommandBuffers();
}

FirstApp::~FirstApp() {
  vkDestroyPipelineLayout(vaDevice.device(), pipelineLayout, nullptr);
}

void FirstApp::drawFrame() {}

void FirstApp::createCommandBuffers() {}

void FirstApp::run() {
  while (!vaWindow.shouldClose()) {
    glfwPollEvents();
  }
}

void FirstApp::createPipelineLayout() {
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pSetLayouts = nullptr,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr};

  if (vkCreatePipelineLayout(vaDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Could not create pipeline layout");
  }
};

void FirstApp::createPipeline() {
  PipelineConfigInfo configInfo{};
  VaPipeline::setDefaultPipelineConfigInfo(configInfo, vaSwapChain.width(),
                                           vaSwapChain.height());
  configInfo.renderPass = vaSwapChain.getRenderPass();
  configInfo.pipelineLayout = pipelineLayout;
  vaPipeline = std::make_unique<VaPipeline>(
      vaDevice, "./shaders/simple_shader.vert.spv",
      "./shaders/simple_shader.frag.spv", configInfo);
};

} // namespace va
