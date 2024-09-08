#pragma once

#include "va_descriptors.hpp"
#include "va_engine_device.hpp"
#include "va_game_object.hpp"
#include "va_renderer.hpp"
#include "va_window.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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
  void loadGameObjects();

  VaWindow vaWindow{WIDTH, HEIGHT, "helloworld"};
  VaDevice vaDevice{vaWindow};
  VaRenderer vaRenderer{vaWindow, vaDevice};

  std::unique_ptr<VaDescriptorPool> globalPool;
  std::vector<VaGameObject> gameObjects;
};
} // namespace va
