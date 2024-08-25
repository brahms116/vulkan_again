#pragma once

#include "va_engine_device.hpp"
#include "va_model.hpp"
#include "va_pipeline.hpp"
#include "va_swap_chain.hpp"
#include "va_window.hpp"

#include <memory>

namespace va {

class FirstApp {
public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  FirstApp();
  ~FirstApp();

  FirstApp(const FirstApp &) = delete;
  FirstApp &operator=(const FirstApp &) = delete;

  void run();

private:
  void loadModels();
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void recreateSwapChain();
  void freeCommandBuffers();
  void recordCommandBuffer(int imageIndex);

  void drawFrame();

  VaWindow vaWindow{WIDTH, HEIGHT, "helloworld"};
  VaDevice vaDevice{vaWindow};

  std::unique_ptr<VaSwapChain> vaSwapChain;
  std::unique_ptr<VaPipeline> vaPipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkCommandBuffer> commandBuffers;
  std::unique_ptr<VaModel> vaModel;
};
} // namespace va
