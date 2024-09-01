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

  bool wasWindowResized() { return frameBufferResized; };

  void resetWindowResizedFlag() { frameBufferResized = false; };

  VkExtent2D getExtent() {
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
  }

  bool shouldClose() { return glfwWindowShouldClose(window); }

  GLFWwindow *getGLFWwindow() const { return window; };

  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

private:
  GLFWwindow *window;

  bool frameBufferResized = false;

  static void onWindowResized(GLFWwindow *pointer, int width, int height);

  void initWindow();
  int width;
  int height;

  std::string windowName;
};
} // namespace va
