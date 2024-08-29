#include "first_app.hpp"

#include <array>
#include <stdexcept>

namespace va {

FirstApp::FirstApp() { loadGameObjects(); }

FirstApp::~FirstApp() {}

void FirstApp::loadGameObjects() {
  std::vector<VaModel::Vertex> vertices{
      {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
  };

  auto model = std::make_shared<VaModel>(vaDevice, vertices);
  auto triangle = VaGameObject::create();
  triangle.model = model;
  triangle.color = {1.f, 1.f, 1.f, 1.f};
  triangle.transform2d.translation.x = .5f;
  triangle.transform2d.translation.y = .0f;
  triangle.transform2d.scale = {1.f, 1.f};
  triangle.transform2d.rotation = 0.25 * glm::two_pi<float>();

  gameObjects.push_back(std::move(triangle));
}

void FirstApp::run() {
  SimpleRenderSystem simpleRenderSystem{vaDevice,
                                        vaRenderer.getSwapChainRenderPass()};
  while (!vaWindow.shouldClose()) {
    glfwPollEvents();
    if (auto commandBuffer = vaRenderer.beginFrame()) {
      vaRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
      vaRenderer.endSwapChainRenderPass(commandBuffer);
      vaRenderer.endFrame();
    };

    vkDeviceWaitIdle(vaDevice.device());
  }
}

} // namespace va
