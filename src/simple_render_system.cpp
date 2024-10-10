#include "simple_render_system.hpp"

#include <array>
#include <cmath>
#include <stdexcept>

namespace va {

SimpleRenderSystem::SimpleRenderSystem(VaDevice &device,
                                       VkRenderPass renderPass,
                                       VkDescriptorSetLayout globalSetLayout,
                                       VkDescriptorSetLayout textureSetLayout)
    : vaDevice{device} {
  createPipelineLayout(globalSetLayout, textureSetLayout);
  createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
  vkDestroyPipelineLayout(vaDevice.device(), pipelineLayout, nullptr);
}

void SimpleRenderSystem::renderGameObjects(FrameInfo &frameInfo) {
  vaPipeline->bindCommandBuffer(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                          &frameInfo.descriptorSet, 0, nullptr);

  for (auto &kv : frameInfo.gameObjects) {

    auto &object = kv.second;

    if (object.model == nullptr)
      continue;

    vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1,
                            1, &object.model->textureDescriptor, 0, nullptr);

    SimplePushConstantData push{};
    /* object.transform.scale.x = */
    /*     3.0 + fmod((object.transform.scale.x + 0.01f), 1); */
    auto modelTransform = object.transform.mat4();
    push.modelMatrix = modelTransform;
    push.normalMatrix = object.transform.normalMatrix();
    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT |
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(SimplePushConstantData), &push);

    object.model->bindCommandBuffer(frameInfo.commandBuffer);
    object.model->draw(frameInfo.commandBuffer);
  }
}

void SimpleRenderSystem::createPipelineLayout(
    VkDescriptorSetLayout globalSetLayout,
    VkDescriptorSetLayout textureSetLayout) {
  VkPushConstantRange push{};
  push.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push.offset = 0;
  push.size = sizeof(SimplePushConstantData);

  std::array<VkDescriptorSetLayout, 2> descriptorSets = {globalSetLayout,
                                                         textureSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = static_cast<uint32_t>(descriptorSets.size()),
      .pSetLayouts = descriptorSets.data(),
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
