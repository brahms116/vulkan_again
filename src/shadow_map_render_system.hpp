#include "va_engine_device.hpp"
#include "va_game_object.hpp"
#include "va_pipeline.hpp"
#include "va_texture.hpp"
namespace va {

class ShadowMapRenderSystem {

  struct PushConstantData {
    glm::mat4 modelMatrix{1.f};
  };

public:
  ShadowMapRenderSystem(VaDevice &vaDevice, VkExtent2D shadowMapExtent);

  ~ShadowMapRenderSystem();

  void renderShadowMap(VkCommandBuffer commandBuffer,
                       const VaGameObject::Map &gameObjects, int frameIndex);

  VkDescriptorImageInfo descriptorInfo(int frameIndex) const;

private:
  VkExtent2D shadowMapExtent;
  VaDevice &vaDevice;
  VkRenderPass renderPass;
  std::unique_ptr<VaPipeline> vaPipeline;
  VkPipelineLayout pipelineLayout;

  std::vector<std::unique_ptr<VaTexture>> depthImages;
  std::vector<VkFramebuffer> framebuffers;

  void initializeDepthImages();
  void initializeRenderPass();
  void initializeFramebuffers();
  void initializePipeline();
};
} // namespace va
