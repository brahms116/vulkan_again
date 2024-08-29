#include "first_app.hpp"

#include <stdexcept>

#include <array>

namespace va {

FirstApp::FirstApp() {
  loadGameObjects();
  createPipelineLayout();
  createPipeline();
}

FirstApp::~FirstApp() {
  vkDestroyPipelineLayout(vaDevice.device(), pipelineLayout, nullptr);
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
  triangle.color = {1.f, 1.f, 1.f, 1.f};
  triangle.transform2d.translation.x = .5f;
  triangle.transform2d.translation.y = .0f;
  triangle.transform2d.scale = {1.f, 1.f};
  triangle.transform2d.rotation = 0.25 * glm::two_pi<float>();

  gameObjects.push_back(std::move(triangle));
}

void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer) {
  vaPipeline->bindCommandBuffer(commandBuffer);

  for (const auto &object : gameObjects) {
    SimplePushConstantData push{};
    push.transform = object.transform2d.mat2();
    push.offset = object.transform2d.translation;
    push.color = object.color;
    vkCmdPushConstants(commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT |
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(SimplePushConstantData), &push);

    object.model->bindCommandBuffer(commandBuffer);
    object.model->draw(commandBuffer);
  }
}

void FirstApp::run() {
  while (!vaWindow.shouldClose()) {
    glfwPollEvents();
    if (auto commandBuffer = vaRenderer.beginFrame()) {
      vaRenderer.beginSwapChainRenderPass(commandBuffer);
      renderGameObjects(commandBuffer);
      vaRenderer.endSwapChainRenderPass(commandBuffer);
      vaRenderer.endFrame();
    };

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
  configInfo.renderPass = vaRenderer.getSwapChainRenderPass();
  configInfo.pipelineLayout = pipelineLayout;
  vaPipeline = std::make_unique<VaPipeline>(
      vaDevice, "./shaders/simple_shader.vert.spv",
      "./shaders/simple_shader.frag.spv", configInfo);
};

} // namespace va
