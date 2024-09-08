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

class SimpleRenderSystem {

  struct SimplePushConstantData {
    glm::mat4 transform{1.f};
    glm::mat4 normalMatrix{1.f};
  };

public:
  SimpleRenderSystem(VaDevice &device, VkRenderPass renderPass);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;
  void renderGameObjects(const FrameInfo &frameInfor,
                         std::vector<VaGameObject> &gameObjects);

  void run();

private:
  void loadGameObjects();
  void createPipelineLayout();
  void createPipeline(VkRenderPass);

  VaDevice &vaDevice;

  std::unique_ptr<VaPipeline> vaPipeline;
  VkPipelineLayout pipelineLayout;
};
} // namespace va
