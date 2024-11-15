#include "va_pipeline.hpp"
#include "va_model.hpp"

#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>

namespace va {

VaPipeline::VaPipeline(VaDevice &device,
                       const std::string &vertexShaderFilePath,
                       const std::string &fragmentShaderFilePath,
                       const PipelineConfigInfo &configInfo)
    : vaDevice{device} {
  createGraphicsPipeline(vertexShaderFilePath, fragmentShaderFilePath,
                         configInfo);
}

VaPipeline::VaPipeline(VaDevice &device,
                       const std::string &vertexShaderFilePath,
                       const PipelineConfigInfo &configInfo)
    : vaDevice{device} {
  createGraphicsPipeline(vertexShaderFilePath, std::nullopt, configInfo);
}

VaPipeline::~VaPipeline() {
  if (vertShaderModule != VK_NULL_HANDLE) {
    vkDestroyShaderModule(vaDevice.device(), vertShaderModule, nullptr);
  }
  if (fragShaderModule != VK_NULL_HANDLE) {
    vkDestroyShaderModule(vaDevice.device(), fragShaderModule, nullptr);
  }
  vkDestroyPipeline(vaDevice.device(), graphicsPipeline, nullptr);
}

void VaPipeline::bindCommandBuffer(VkCommandBuffer commandBuffer) {
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    graphicsPipeline);
}

std::vector<char> VaPipeline::readFile(const std::string &filePath) {

  std::ifstream file{filePath, std::ios::ate | std::ios::binary};

  if (!file.is_open()) {
    throw std::runtime_error("fialed to open file: " + filePath);
  }

  size_t fileSize = static_cast<size_t>(file.tellg());

  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();
  return buffer;
};

void VaPipeline::createGraphicsPipeline(
    const std::optional<std::string> vertexShaderFilePath,
    const std::optional<std::string> fragmentShaderFilePath,
    const PipelineConfigInfo &configInfo) {

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

  if (vertexShaderFilePath.has_value()) {
    auto vertCode = readFile(vertexShaderFilePath.value());
    createShaderModule(vertCode, &vertShaderModule);
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
    };
    shaderStages.push_back(vertShaderStageInfo);
  }

  if (fragmentShaderFilePath.has_value()) {
    auto fragCode = readFile(fragmentShaderFilePath.value());
    createShaderModule(fragCode, &fragShaderModule);
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
    };
    shaderStages.push_back(fragShaderStageInfo);
  }

  auto &bindingDescriptions = configInfo.bindingDescriptions;
  auto &attributeDescriptions = configInfo.attributeDescriptions;

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount =
          static_cast<uint32_t>(bindingDescriptions.size()),
      .pVertexBindingDescriptions = bindingDescriptions.data(),
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(attributeDescriptions.size()),
      .pVertexAttributeDescriptions = attributeDescriptions.data(),
  };

  auto multisampleInfo = configInfo.multisampleInfo;
  if (configInfo.useDeviceMsaaSamples) {
    multisampleInfo.rasterizationSamples = vaDevice.msaaSampleCount;
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = static_cast<uint32_t>(shaderStages.size()),
      .pStages = shaderStages.data(),
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &configInfo.inputAssemblyInfo,
      .pViewportState = &configInfo.viewportInfo,
      .pRasterizationState = &configInfo.rasterizationInfo,
      .pMultisampleState = &multisampleInfo,
      .pDepthStencilState = &configInfo.depthStencilInfo,
      .pColorBlendState = &configInfo.colorBlendInfo,
      .pDynamicState = &configInfo.dynamicStateInfo,
      .layout = configInfo.pipelineLayout,
      .renderPass = configInfo.renderPass,
      .subpass = configInfo.subpass,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
  };

  if (vkCreateGraphicsPipelines(vaDevice.device(), VK_NULL_HANDLE, 1,
                                &pipelineInfo, nullptr,
                                &graphicsPipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline");
  }
}
void VaPipeline::createShaderModule(const std::vector<char> &code,
                                    VkShaderModule *shaderModule) {
  VkShaderModuleCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = code.size(),
      .pCode = reinterpret_cast<const uint32_t *>(code.data()),
  };

  if (vkCreateShaderModule(vaDevice.device(), &createInfo, nullptr,
                           shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module");
  }
}

void VaPipeline::setDefaultPipelineConfigInfo(PipelineConfigInfo &configInfo) {

  configInfo.inputAssemblyInfo = VkPipelineInputAssemblyStateCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE};

  configInfo.viewportInfo = VkPipelineViewportStateCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = nullptr,
      .scissorCount = 1,
      .pScissors = nullptr};

  configInfo.rasterizationInfo = VkPipelineRasterizationStateCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_NONE,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0,
  };

  configInfo.multisampleInfo = VkPipelineMultisampleStateCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
  };

  configInfo.colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
  configInfo.colorBlendAttachment.srcColorBlendFactor =
      VK_BLEND_FACTOR_ONE; // Optional
  configInfo.colorBlendAttachment.dstColorBlendFactor =
      VK_BLEND_FACTOR_ZERO;                                       // Optional
  configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
  configInfo.colorBlendAttachment.srcAlphaBlendFactor =
      VK_BLEND_FACTOR_ONE; // Optional
  configInfo.colorBlendAttachment.dstAlphaBlendFactor =
      VK_BLEND_FACTOR_ZERO;                                       // Optional
  configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

  configInfo.colorBlendInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
  configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
  configInfo.colorBlendInfo.attachmentCount = 1;
  configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
  configInfo.colorBlendInfo.blendConstants[0] = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[1] = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[2] = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[3] = 0.0f; // Optional

  configInfo.depthStencilInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.minDepthBounds = 0.0f; // Optional
  configInfo.depthStencilInfo.maxDepthBounds = 1.0f; // Optional
  configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.front = {}; // Optional
  configInfo.depthStencilInfo.back = {};  // Optional

  configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,
                                    VK_DYNAMIC_STATE_SCISSOR};
  configInfo.dynamicStateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  configInfo.dynamicStateInfo.pDynamicStates =
      configInfo.dynamicStateEnables.data();
  configInfo.dynamicStateInfo.dynamicStateCount =
      static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
  configInfo.dynamicStateInfo.flags = 0;

  configInfo.bindingDescriptions = VaModel::Vertex::getBindingDescriptions();
  configInfo.attributeDescriptions =
      VaModel::Vertex::getAttributeDescriptions();
}

} // namespace va
