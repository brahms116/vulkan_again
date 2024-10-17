
#include "va_engine_device.hpp"
#include "va_swap_chain.hpp"
#include "va_texture.hpp"
namespace va {

class ShadowMapRenderSystem {

public:
  ShadowMapRenderSystem(VaDevice &vaDevice, VkExtent2D swapChainExtent);

private:
  VaDevice &vaDevice;
  VkFramebuffer framebuffer;
  VaTexture depthImage;
};
} // namespace va
