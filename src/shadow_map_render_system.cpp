#include "shadow_map_render_system.hpp"
#include "va_swap_chain.hpp"
#include <array>
#include <stdexcept>

namespace va {

ShadowMapRenderSystem::ShadowMapRenderSystem(VaDevice &vaDevice,
                                             VkExtent2D shadowMapExtent)
    : vaDevice(vaDevice), shadowMapExtent(shadowMapExtent) {
  initializeDepthImages();
  initializeRenderPass();
  initializeFramebuffers();
  initializePipeline();
}

ShadowMapRenderSystem::~ShadowMapRenderSystem() {
  for (auto framebuffer : framebuffers) {
    vkDestroyFramebuffer(vaDevice.device(), framebuffer, nullptr);
  }
  vkDestroyRenderPass(vaDevice.device(), renderPass, nullptr);
}

void ShadowMapRenderSystem::initializeDepthImages() {
  auto createImageProperties = VaTexture::CreateImageProperties{
      shadowMapExtent.width,
      shadowMapExtent.height,
      VK_FORMAT_D16_UNORM,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_IMAGE_ASPECT_DEPTH_BIT};

  depthImages.resize(VaSwapChain::MAX_FRAMES_IN_FLIGHT);

  for (int i = 0; i < VaSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
    depthImages[i] =
        std::make_unique<VaTexture>(vaDevice, createImageProperties);
  }
}

void ShadowMapRenderSystem::initializeRenderPass() {
  VkAttachmentDescription ad{};
  ad.format = VK_FORMAT_D16_UNORM;
  ad.samples = VK_SAMPLE_COUNT_1_BIT;
  ad.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  ad.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  ad.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  ad.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  ad.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  ad.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

  VkAttachmentReference ar{};
  ar.attachment = 0;
  ar.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription sd{};
  sd.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  sd.colorAttachmentCount = 0;
  sd.pDepthStencilAttachment = &ar;

  std::array<VkSubpassDependency, 2> sbDeps;
  sbDeps[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  sbDeps[0].dstSubpass = 0;
  sbDeps[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  sbDeps[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  sbDeps[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  sbDeps[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  sbDeps[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  sbDeps[1].srcSubpass = 0;
  sbDeps[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  sbDeps[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  sbDeps[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  sbDeps[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  sbDeps[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  sbDeps[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo rpi{};
  rpi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  rpi.attachmentCount = 1;
  rpi.pAttachments = &ad;
  rpi.subpassCount = 1;
  rpi.pSubpasses = &sd;
  rpi.dependencyCount = static_cast<uint32_t>(sbDeps.size());
  rpi.pDependencies = sbDeps.data();

  if (vkCreateRenderPass(vaDevice.device(), &rpi, nullptr, &renderPass) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create render pass");
  }
}

void ShadowMapRenderSystem::initializeFramebuffers() {
  framebuffers.resize(VaSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < VaSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
    auto imageView = depthImages[i]->getImageView();
    VkFramebufferCreateInfo fbi{};
    fbi.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbi.renderPass = renderPass;
    fbi.attachmentCount = 1;
    fbi.pAttachments = &imageView;
    fbi.width = shadowMapExtent.width;
    fbi.height = shadowMapExtent.height;
    fbi.layers = 1;

    if (vkCreateFramebuffer(vaDevice.device(), &fbi, nullptr,
                            &framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffer");
    }
  }
}

void ShadowMapRenderSystem::initializePipeline() {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(PushConstantData);

  // TODO: pass in the descriptor set layout

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

  if (vkCreatePipelineLayout(vaDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout");
  }

  PipelineConfigInfo configInfo{};
  VaPipeline::setDefaultPipelineConfigInfo(configInfo);
  configInfo.pipelineLayout = pipelineLayout;
  configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  configInfo.rasterizationInfo.depthBiasEnable = VK_TRUE;
  configInfo.rasterizationInfo.depthBiasConstantFactor = 1.25f;
  configInfo.rasterizationInfo.depthBiasSlopeFactor = 1.75f;
  configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  configInfo.renderPass = renderPass;
  vaPipeline = std::make_unique<VaPipeline>(
      vaDevice, "./shaders/shadow.vert.spv", configInfo);
}

void ShadowMapRenderSystem::renderShadowMap(
    VkCommandBuffer commandBuffer, const VaGameObject::Map &gameObjects,
    int frameIndex) {

  VkRenderPassBeginInfo renderPassBeginInfo{};
  renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassBeginInfo.renderPass = renderPass;
  renderPassBeginInfo.framebuffer = framebuffers[frameIndex];
  renderPassBeginInfo.renderArea.extent = shadowMapExtent;

  std::array<VkClearValue, 1> clearValues{{1.0f, 0}};
  renderPassBeginInfo.clearValueCount =
      static_cast<uint32_t>(clearValues.size());
  renderPassBeginInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(shadowMapExtent.width);
  viewport.height = static_cast<float>(shadowMapExtent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scisscor{{0, 0}, shadowMapExtent};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scisscor);
  vaPipeline->bindCommandBuffer(commandBuffer);

  // Render each game object

  // End the render pass
  vkCmdEndRenderPass(commandBuffer);
}

VkDescriptorImageInfo
ShadowMapRenderSystem::descriptorInfo(int frameIndex) const {
  return depthImages[frameIndex]->descriptorInfo(
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
}

} // namespace va
