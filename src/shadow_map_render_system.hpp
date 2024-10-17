
#include "va_engine_device.hpp"
#include "va_swap_chain.hpp"
#include "va_texture.hpp"
namespace va {

class ShadowMapRenderSystem {

public:
  ShadowMapRenderSystem(VaDevice &vaDevice, VaSwapChain &vaSwapChain);

private:
  VaDevice &vaDevice;
  VaSwapChain &vaSwapChain;
  VkFramebuffer framebuffer;

  VaTexture depthImage{
      vaDevice,
      VaTexture::CreateImageProperties{
          vaSwapChain.width(), vaSwapChain.height(),
          vaSwapChain.getSwapChainDepthFormat(), VK_IMAGE_TILING_OPTIMAL,
          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
              VK_IMAGE_USAGE_SAMPLED_BIT,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT},

  };
};
} // namespace va
