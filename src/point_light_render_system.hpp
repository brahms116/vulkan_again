#pragma once

#include "va_engine_device.hpp"
#include "va_frame_info.hpp"
#include "va_pipeline.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>

namespace va {

class PointLightRenderSystem {

  struct PushConstantData {
    glm::vec4 position;
    glm::vec4 color;
    float radius;
  };

public:
  PointLightRenderSystem(VaDevice &device, VkRenderPass renderPass,
                         VkDescriptorSetLayout globalSetLayout);
  ~PointLightRenderSystem();

  PointLightRenderSystem(const PointLightRenderSystem &) = delete;
  PointLightRenderSystem &operator=(const PointLightRenderSystem &) = delete;

  void update(FrameInfo &frameInfo, GlobalUbo &ubo);
  void render(const FrameInfo &frameInfo);

  void run();

private:
  void loadGameObjects();
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass);

  VaDevice &vaDevice;

  std::unique_ptr<VaPipeline> vaPipeline;
  VkPipelineLayout pipelineLayout;
};
} // namespace va
