#include "shadow_map_render_system.hpp"
namespace va {

ShadowMapRenderSystem::ShadowMapRenderSystem(VaDevice &vaDevice,
                                             VkExtent2D swapChainExtent,
                                             VkFormat swapChainDepthFormat)
    : vaDevice(vaDevice),
      depthImage(vaDevice, VaTexture::CreateImageProperties{
                               swapChainExtent.width, swapChainExtent.height,
                               swapChainDepthFormat, VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                   VK_IMAGE_USAGE_SAMPLED_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               VK_IMAGE_ASPECT_DEPTH_BIT}) {}

} // namespace va
