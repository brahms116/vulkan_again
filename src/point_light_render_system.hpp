#pragma once

#include "va_engine_device.hpp"
#include "va_frame_info.hpp"
#include "va_game_object.hpp"
#include "va_pipeline.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "va_camera.hpp"

#include <memory>

namespace va {

class PointLightRenderSystem {

  struct SimplePushConstantData {
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
  };

public:
  PointLightRenderSystem(VaDevice &device, VkRenderPass renderPass,
                     VkDescriptorSetLayout globalSetLayout);
  ~PointLightRenderSystem();

  PointLightRenderSystem(const PointLightRenderSystem &) = delete;
  PointLightRenderSystem &operator=(const PointLightRenderSystem &) = delete;
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
