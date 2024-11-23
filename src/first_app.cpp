#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "point_light_render_system.hpp"
#include "shadow_map_render_system.hpp"
#include "simple_render_system.hpp"
#include "va_buffer.hpp"
#include "va_frame_info.hpp"
#include "va_model.hpp"
#include "va_texture.hpp"

#include <chrono>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

namespace va {

FirstApp::FirstApp() {
  globalPool = VaDescriptorPool::Builder(vaDevice)
                   .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                VaSwapChain::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                VaSwapChain::MAX_FRAMES_IN_FLIGHT + 2)
                   .setMaxSets(VaSwapChain::MAX_FRAMES_IN_FLIGHT * 2 + 2)
                   .build();
}

FirstApp::~FirstApp() {}

void FirstApp::loadGameObjects(const VkDescriptorSet defaultTextureDescriptor,
                               const VkDescriptorSet floorTextureDescriptor) {

  std::shared_ptr<VaModel> cubeModel = VaModel::createModelFromFile(
      vaDevice, "models/cube.obj", defaultTextureDescriptor);

  std::shared_ptr<VaModel> vaseModel = VaModel::createModelFromFile(
      vaDevice, "models/smooth_vase.obj", defaultTextureDescriptor);

  std::shared_ptr<VaModel> floorModel = VaModel::createModelFromFile(
      vaDevice, "models/quad.obj", floorTextureDescriptor);

  auto thing = VaGameObject::create();
  thing.model = cubeModel;
  thing.transform.translation = {0.5f, 0.f, 0.f};
  thing.transform.scale = glm::vec3(0.5f);
  gameObjects.emplace(thing.getId(), std::move(thing));

  auto vase = VaGameObject::create();
  vase.model = vaseModel;
  vase.transform.translation = {3.f, 0.5f, 2.5f};
  vase.transform.scale = glm::vec3(8.f);
  gameObjects.emplace(vase.getId(), std::move(vase));

  auto vase1 = VaGameObject::create();
  vase1.model = vaseModel;
  vase1.transform.translation = {4.2f, .5f, 1.15f};
  vase1.transform.scale = glm::vec3(6.f);
  gameObjects.emplace(vase1.getId(), std::move(vase1));

  auto floor = VaGameObject::create();
  floor.model = floorModel;
  floor.transform.translation = {0.f, 0.5f, 0.f};
  floor.transform.scale = glm::vec3(10.f, 1.f, 10.f);
  floor.isQuad = true;
  gameObjects.emplace(floor.getId(), std::move(floor));

  auto pointLight = VaGameObject::makePointLight(.01f, {1.0f, 1.0f, 1.0f, .5f});
  pointLight.transform.translation = {35.f, -10.0f, 0.f};
  gameObjects.emplace(pointLight.getId(), std::move(pointLight));
}

void FirstApp::run() {

  auto globalDescriptorSetLayout =
      VaDescriptorSetLayout::Builder(vaDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                      VK_SHADER_STAGE_ALL_GRAPHICS)
          .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                      VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  auto textureDescriptorSetLayout =
      VaDescriptorSetLayout::Builder(vaDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                      VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  VkDescriptorSet defaultTextureDescriptorSet;
  auto defaultTextureDescriptorInfo = defaultTexture.descriptorInfo();

  VaDescriptorWriter(*textureDescriptorSetLayout, *globalPool)
      .writeImage(0, &defaultTextureDescriptorInfo)
      .build(defaultTextureDescriptorSet);

  VkDescriptorSet floorTextureDescriptorSet;
  auto floorTextureDescriptorInfo = floorTexture.descriptorInfo();

  VaDescriptorWriter(*textureDescriptorSetLayout, *globalPool)
      .writeImage(0, &floorTextureDescriptorInfo)
      .build(floorTextureDescriptorSet);

  loadGameObjects(defaultTextureDescriptorSet, floorTextureDescriptorSet);

  ShadowMapRenderSystem shadowRenderSystem{
      vaDevice, VkExtent2D{2400, 1800},
      globalDescriptorSetLayout->getDescriptorSetLayout()};

  SimpleRenderSystem simpleRenderSystem{
      vaDevice, vaRenderer.getSwapChainRenderPass(),
      globalDescriptorSetLayout->getDescriptorSetLayout(),
      textureDescriptorSetLayout->getDescriptorSetLayout()};

  PointLightRenderSystem pointLightRenderSystem{
      vaDevice, vaRenderer.getSwapChainRenderPass(),
      globalDescriptorSetLayout->getDescriptorSetLayout()};

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

  std::vector<VkDescriptorSet> globalDescriptorSets(
      VaSwapChain::MAX_FRAMES_IN_FLIGHT);

  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uniformBuffers[i]->descriptorInfo();
    auto imageInfo = shadowRenderSystem.descriptorInfo(i);
    VaDescriptorWriter(*globalDescriptorSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .writeImage(1, &imageInfo)
        .build(globalDescriptorSets[i]);
  }

  VaCamera camera{};
  auto cameraEmpty = VaGameObject::create();
  cameraEmpty.transform.translation.z = -2.5f;

  VaCamera lightCamera{};

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
    camera.setPerspectiveProjection(1.4f, aspectRatio, 0.1f, 100.f);
    lightCamera.setPerspectiveProjection(45.f, aspectRatio, 1.1f, 100.f);

    inputController.updateTransformXZ(vaWindow.getGLFWwindow(), cameraEmpty,
                                      dt);
    camera.setViewYXZ(cameraEmpty.transform.translation,
                      cameraEmpty.transform.rotation);

    GlobalUbo ubo{};

    // Loop through all the game objects, find the point light and set its
    // position as the light camera position then set the target to be the
    // origin
    for (auto &kv : gameObjects) {
      auto &o = kv.second;
      if (o.pointLightComponent != nullptr) {
        lightCamera.setViewTarget(o.transform.translation, {0.f, 0.f, 0.f});

        ubo.lightProjectionMatrix =
            glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 96.f);
        ubo.lightViewMatrix = glm::lookAt(o.transform.translation,
                                          {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f});
      }
    }

    if (auto commandBuffer = vaRenderer.beginFrame()) {
      // Update
      int frameIndex = vaRenderer.getFrameIndex();

      FrameInfo frameInfo{frameIndex,
                          dt,
                          commandBuffer,
                          camera,
                          globalDescriptorSets[frameIndex],
                          gameObjects};

      ubo.projectionMatrix = camera.getProjection();
      ubo.viewMatrix = camera.getView();

      /* ubo.lightProjectionMatrix = lightCamera.getProjection(); */
      /* ubo.lightViewMatrix = lightCamera.getView(); */

      ubo.inverseViewMatrix = cameraEmpty.transform.mat4();
      ubo.inverseViewMatrix = cameraEmpty.transform.mat4();

      pointLightRenderSystem.update(frameInfo, ubo);

      uniformBuffers[frameIndex]->writeToBuffer(&ubo);
      uniformBuffers[frameIndex]->flush();

      // Render
      shadowRenderSystem.renderShadowMap(frameInfo);

      vaRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(frameInfo);
      /* pointLightRenderSystem.render(frameInfo); */
      vaRenderer.endSwapChainRenderPass(commandBuffer);
      vaRenderer.endFrame();
    };
  }
  vkDeviceWaitIdle(vaDevice.device());
}

} // namespace va
