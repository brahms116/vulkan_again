#include "va_engine_device.hpp"
#include "va_game_object.hpp"
#include "va_pipeline.hpp"
#include "va_texture.hpp"
#include "va_frame_info.hpp"

namespace va {

class ShadowMapRenderSystem {

  struct PushConstantData {
    glm::mat4 modelMatrix{1.f};
  };

public:
  ShadowMapRenderSystem(VaDevice &vaDevice, VkExtent2D shadowMapExtent,
                        VkDescriptorSetLayout globalSetLayout);

  ~ShadowMapRenderSystem();

  void renderShadowMap(FrameInfo frameInfo);

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
  void initializePipeline(VkDescriptorSetLayout globalSetLayout);
};
} // namespace va
