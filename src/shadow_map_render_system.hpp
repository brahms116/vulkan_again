
#include "va_engine_device.hpp"
#include "va_texture.hpp"
#include "va_pipeline.hpp"
namespace va {

class ShadowMapRenderSystem {

public:
  ShadowMapRenderSystem(VaDevice &vaDevice, VkExtent2D shadowMapExtent);

private:
  VkExtent2D shadowMapExtent;
  VaDevice &vaDevice;
  VkFramebuffer framebuffer;
  VaTexture depthImage;
  VkRenderPass renderPass;
  VaPipeline pipeline;


  void initializeRenderPass();
  void initializeFramebuffer();
};
} // namespace va
