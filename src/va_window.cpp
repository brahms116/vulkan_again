#include "va_window.hpp"

#include <stdexcept>

namespace va {

VaWindow::VaWindow(int w, int h, std::string name)
    : width(w), height(h), windowName(name) {
  initWindow();
}

VaWindow::~VaWindow() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void VaWindow::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window =
      glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, onWindowResized);
}

void VaWindow::onWindowResized(GLFWwindow *pointer, int width, int height) {
  VaWindow *window = reinterpret_cast<VaWindow *>(pointer);
  window->frameBufferResized = true;
  window->width = width;
  window->height = height;
}

void VaWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface");
  }
}
} // namespace va
