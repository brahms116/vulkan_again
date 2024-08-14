#include "first_app.hpp"

namespace va {
void FirstApp::run() {
  while (!vaWindow.shouldClose()) {
    glfwPollEvents();
  }
}
} // namespace va
