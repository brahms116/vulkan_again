#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "simple_render_system.hpp"
#include "va_model.hpp"

#include <chrono>

namespace va {

FirstApp::FirstApp() { loadGameObjects(); }

FirstApp::~FirstApp() {}

void FirstApp::loadGameObjects() {

  std::shared_ptr<VaModel> cubeModel = VaModel::createModelFromFile(vaDevice, "models/smooth_vase.obj");

  auto thing = VaGameObject::create();
  thing.model = cubeModel;
  thing.transform.translation = {0.f, 0.f, 2.5f};
  thing.transform.scale = glm::vec3(3.f);
  gameObjects.push_back(std::move(thing));
}

void FirstApp::run() {
  SimpleRenderSystem simpleRenderSystem{vaDevice,
                                        vaRenderer.getSwapChainRenderPass()};
  VaCamera camera{};
  auto cameraEmpty = VaGameObject::create();
  auto currentTime = std::chrono::high_resolution_clock::now();

  KeyboardMovementController inputController{};

  while (!vaWindow.shouldClose()) {
    glfwPollEvents();
    auto newTime = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float, std::chrono::seconds::period>(
                   newTime - currentTime)
                   .count();

    dt = std::min(dt, 0.1f);

    currentTime = newTime;
    auto aspectRatio = vaRenderer.getAspectRatio();
    camera.setPerspectiveProjection(1.4f, aspectRatio, 0.1f, 5.f);
    inputController.updateTransformXZ(vaWindow.getGLFWwindow(), cameraEmpty,
                                      dt);

    camera.setViewYXZ(cameraEmpty.transform.translation,
                      cameraEmpty.transform.rotation);

    if (auto commandBuffer = vaRenderer.beginFrame()) {
      vaRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
      vaRenderer.endSwapChainRenderPass(commandBuffer);
      vaRenderer.endFrame();
    };

    vkDeviceWaitIdle(vaDevice.device());
  }
}

} // namespace va
