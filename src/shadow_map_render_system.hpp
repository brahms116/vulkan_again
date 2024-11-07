
#include "va_engine_device.hpp"
#include "va_texture.hpp"
#include "va_pipeline.hpp"
namespace va {

class ShadowMapRenderSystem {

public:
  ShadowMapRenderSystem(VaDevice &vaDevice, VkExtent2D shadowMapExtent);

  ~ShadowMapRenderSystem();

private:
  VkExtent2D shadowMapExtent;
  VaDevice &vaDevice;
  VkFramebuffer framebuffer;
  VaTexture depthImage;
  VkRenderPass renderPass;
  std::unique_ptr<VaPipeline> vaPipeline;

  void initializeRenderPass();
  void initializeFramebuffer();
  void initializePipeline();
};
} // namespace va
