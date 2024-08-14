#pragma once

#include "va_window.hpp"

namespace va {

class FirstApp {
public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  void run();

private:
  VaWindow vaWindow{WIDTH, HEIGHT, "helloworld"};
};
} // namespace va
