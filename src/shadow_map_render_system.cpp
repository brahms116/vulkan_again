#include "shadow_map_render_system.hpp"
namespace va {

ShadowMapRenderSystem::ShadowMapRenderSystem(VaDevice &vaDevice,
                                             VaSwapChain &vaSwapChain)
    : vaDevice(vaDevice), vaSwapChain(vaSwapChain) {}

} // namespace va
