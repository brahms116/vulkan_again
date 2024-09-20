#include "point_light_render_system.hpp"

#include <cmath>
#include <stdexcept>

namespace va {

PointLightRenderSystem::PointLightRenderSystem(
    VaDevice &device, VkRenderPass renderPass,
    VkDescriptorSetLayout globalSetLayout)
    : vaDevice{device} {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

PointLightRenderSystem::~PointLightRenderSystem() {
  vkDestroyPipelineLayout(vaDevice.device(), pipelineLayout, nullptr);
}

void PointLightRenderSystem::render(const FrameInfo &frameInfo) {
  vaPipeline->bindCommandBuffer(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                          &frameInfo.descriptorSet, 0, nullptr);

  vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
}

void PointLightRenderSystem::createPipelineLayout(
    VkDescriptorSetLayout globalSetLayout) {
  /* VkPushConstantRange push{}; */
  /* push.stageFlags = VK_SHADER_STAGE_VERTEX_BIT |
   * VK_SHADER_STAGE_FRAGMENT_BIT; */
  /* push.offset = 0; */
  /* push.size = sizeof(SimplePushConstantData); */

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &globalSetLayout,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr};

  if (vkCreatePipelineLayout(vaDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Could not create pipeline layout");
  }
};

void PointLightRenderSystem::createPipeline(VkRenderPass renderPass) {
  PipelineConfigInfo configInfo{};
  VaPipeline::setDefaultPipelineConfigInfo(configInfo);
  configInfo.renderPass = renderPass;
  configInfo.pipelineLayout = pipelineLayout;
  configInfo.attributeDescriptions.clear();
  configInfo.bindingDescriptions.clear();
  vaPipeline = std::make_unique<VaPipeline>(
      vaDevice, "./shaders/point_light_shader.vert.spv",
      "./shaders/point_light_shader.frag.spv", configInfo);
};

} // namespace va
