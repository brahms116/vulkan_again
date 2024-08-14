#include "va_window.hpp"

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
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window =
      glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}
} // namespace va
