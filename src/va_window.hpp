#pragma once
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <string>

namespace va {
class VaWindow {

public:
  VaWindow(int w, int h, std::string name);
  ~VaWindow();

  VaWindow(const VaWindow &) = delete;
  VaWindow &operator=(const VaWindow &) = delete;

  bool shouldClose() { return glfwWindowShouldClose(window); }

private:
  GLFWwindow *window;

  void initWindow();
  const int width;
  const int height;

  std::string windowName;
};
} // namespace va
