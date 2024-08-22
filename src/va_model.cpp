
#include "va_model.hpp"
#include <cassert>
#include <cstring>

namespace va {

VaModel::VaModel(VaDevice &device, const std::vector<Vertex> &vertices)
    : vaDevice{device} {
  createVertexBuffer(vertices);
};

VaModel::~VaModel() {
  vkDestroyBuffer(vaDevice.device(), vertexBuffer, nullptr);
  vkFreeMemory(vaDevice.device(), vertexBufferMemory, nullptr);
};
void VaModel::createVertexBuffer(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");

  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

  vaDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        vertexBuffer, vertexBufferMemory);

  void *data;
  vkMapMemory(vaDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(vaDevice.device(), vertexBufferMemory);
}
} // namespace va
