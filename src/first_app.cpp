#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "simple_render_system.hpp"
#include "va_buffer.hpp"
#include "va_frame_info.hpp"
#include "va_model.hpp"

#include <chrono>

namespace va {

struct GlobalUbo {
  glm::mat4 projectionView;
  glm::vec4 lightDirection =
      glm::vec4(glm::normalize(glm::vec3(1.f, -3.f, -2.f)), 0.0f);
};

FirstApp::FirstApp() { loadGameObjects(); }

FirstApp::~FirstApp() {}

void FirstApp::loadGameObjects() {

  std::shared_ptr<VaModel> cubeModel =
      VaModel::createModelFromFile(vaDevice, "models/smooth_vase.obj");

  auto thing = VaGameObject::create();
  thing.model = cubeModel;
  thing.transform.translation = {0.f, 0.f, 2.5f};
  thing.transform.scale = glm::vec3(3.f);
  gameObjects.push_back(std::move(thing));
}

void FirstApp::run() {
  SimpleRenderSystem simpleRenderSystem{vaDevice,
                                        vaRenderer.getSwapChainRenderPass()};

  std::vector<std::unique_ptr<VaBuffer>> uniformBuffers(
      VaSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < uniformBuffers.size(); i++) {
    uniformBuffers[i] = std::make_unique<VaBuffer>(
        vaDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vaDevice.properties.limits.minUniformBufferOffsetAlignment);

    uniformBuffers[i]->map();
  }

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
      // Update
      int frameIndex = vaRenderer.getFrameIndex();
      GlobalUbo ubo{};
      ubo.projectionView = camera.getProjection() * camera.getView();
      uniformBuffers[frameIndex]->writeToBuffer(&ubo);
      uniformBuffers[frameIndex]->flush();

      FrameInfo frameInfo{frameIndex, dt, commandBuffer, camera};

      // Render
      vaRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
      vaRenderer.endSwapChainRenderPass(commandBuffer);
      vaRenderer.endFrame();
    };

    vkDeviceWaitIdle(vaDevice.device());
  }
}

} // namespace va
