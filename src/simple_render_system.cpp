#include "simple_render_system.hpp"

#include <cmath>
#include <stdexcept>

namespace va {

SimpleRenderSystem::SimpleRenderSystem(VaDevice &device,
                                       VkRenderPass renderPass)
    : vaDevice{device} {
  createPipelineLayout();
  createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
  vkDestroyPipelineLayout(vaDevice.device(), pipelineLayout, nullptr);
}

void SimpleRenderSystem::renderGameObjects(
    VkCommandBuffer commandBuffer, std::vector<VaGameObject> &gameObjects,
    const VaCamera &camera) {
  vaPipeline->bindCommandBuffer(commandBuffer);

  auto projection = camera.getProjection() * camera.getView();

  for (auto &object : gameObjects) {
    SimplePushConstantData push{};
    object.transform.scale.x =
        3.0 + fmod((object.transform.scale.x + 0.01f), 1);
    auto modelTransform = object.transform.mat4();
    push.transform = projection * modelTransform;
    push.normalMatrix = object.transform.normalMatrix();
    vkCmdPushConstants(commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT |
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(SimplePushConstantData), &push);

    object.model->bindCommandBuffer(commandBuffer);
    object.model->draw(commandBuffer);
  }
}

void SimpleRenderSystem::createPipelineLayout() {
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

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
  PipelineConfigInfo configInfo{};
  VaPipeline::setDefaultPipelineConfigInfo(configInfo);
  configInfo.renderPass = renderPass;
  configInfo.pipelineLayout = pipelineLayout;
  vaPipeline = std::make_unique<VaPipeline>(
      vaDevice, "./shaders/simple_shader.vert.spv",
      "./shaders/simple_shader.frag.spv", configInfo);
};

} // namespace va
