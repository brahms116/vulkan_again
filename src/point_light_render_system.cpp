#include "point_light_render_system.hpp"

#include <cmath>
#include <glm/ext/matrix_transform.hpp>
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
void PointLightRenderSystem::update(FrameInfo &frameInfo, GlobalUbo &ubo) {

  auto rotateLight = glm::rotate(glm::mat4(1.0f), 0.5f * frameInfo.frameTime,
                                 {0.f, -1.f, 0.f});

  int lightIndex = 0;
  for (auto &kv : frameInfo.gameObjects) {
    auto &o = kv.second;
    // Skip if its not a point light object
    if (o.pointLightComponent == nullptr)
      continue;

    o.transform.translation =
        glm::vec3(rotateLight * glm::vec4(o.transform.translation, 1.f));

    ubo.pointLights[lightIndex].color = o.pointLightComponent->color;

    ubo.pointLights[lightIndex].position =
        glm::vec4(o.transform.translation, 1.f);
  }
}

void PointLightRenderSystem::createPipelineLayout(
    VkDescriptorSetLayout globalSetLayout) {
  VkPushConstantRange push{};
  push.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push.offset = 0;
  push.size = sizeof(PushConstantData);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &globalSetLayout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push};

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
