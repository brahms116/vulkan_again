#pragma once

#include "va_engine_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>

namespace va {
class VaModel {
public:
  struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static std::vector<VkVertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();
  };

  VaModel(VaDevice &vaDevice, const std::vector<Vertex> &vertices);
  ~VaModel();

  VaModel(const VaModel &) = delete;
  VaModel &operator=(const VaModel &) = delete;

  void bindCommandBuffer(VkCommandBuffer commandBuffer) const;
  void draw(VkCommandBuffer commandBuffer) const;

private:
  void createVertexBuffer(const std::vector<Vertex> &vertices);
  VaDevice &vaDevice;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  uint32_t vertexCount;
};
} // namespace va
