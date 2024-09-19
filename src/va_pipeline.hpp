#pragma once

#include <string>
#include <vector>

#include "va_engine_device.hpp"

namespace va {

struct PipelineConfigInfo {
  PipelineConfigInfo(const PipelineConfigInfo &) = delete;
  PipelineConfigInfo &operator=(const PipelineConfigInfo &other) = delete;

  PipelineConfigInfo(PipelineConfigInfo &&) = delete;
  PipelineConfigInfo &operator=(const PipelineConfigInfo &&other) = delete;


  std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  VkPipelineLayout pipelineLayout = nullptr;
  VkRenderPass renderPass = nullptr;
  uint32_t subpass = 0;

  std::vector<VkDynamicState> dynamicStateEnables;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
};

class VaPipeline {
public:
  VaPipeline(VaDevice &device, const std::string &vertexShaderFilePath,
             const std::string &fragmentShaderFilePath,
             const PipelineConfigInfo &configInfo);

  ~VaPipeline();

  VaPipeline(const VaPipeline &) = delete;
  void operator=(const VaPipeline &) = delete;

  void bindCommandBuffer(VkCommandBuffer commandBuffer);

  static void setDefaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

private:
  static std::vector<char> readFile(const std::string &filePath);

  void createGraphicsPipeline(const std::string &vertexShaderFilePath,
                              const std::string &fragmentShaderFilePath,
                              const PipelineConfigInfo &configInfo);

  VaDevice &vaDevice;

  void createShaderModule(const std::vector<char> &code,
                          VkShaderModule *shaderModule);

  VkPipeline graphicsPipeline;
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
};
} // namespace va
