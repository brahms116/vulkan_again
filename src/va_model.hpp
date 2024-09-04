#pragma once

#include "va_engine_device.hpp"
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>

namespace va {
class VaModel {
public:
  struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal{};
    glm::vec2 uv{};

    static std::vector<VkVertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();

    bool operator==(const Vertex &other) const {
      return position == other.position && color == other.color &&
             normal == other.normal && uv == other.uv;
    }
  };

  struct Builder {
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};

    void loadModel(const std::string &filepath);
  };

  VaModel(VaDevice &vaDevice, const Builder &builder);
  ~VaModel();

  static std::unique_ptr<VaModel>
  createModelFromFile(VaDevice &device, const std::string &filepath);

  VaModel(const VaModel &) = delete;
  VaModel &operator=(const VaModel &) = delete;

  void bindCommandBuffer(VkCommandBuffer commandBuffer) const;
  void draw(VkCommandBuffer commandBuffer) const;

private:
  bool hasIndexBuffer;

  void createVertexBuffer(const std::vector<Vertex> &vertices);
  void createIndexBuffer(const std::vector<uint32_t> &indices);

  VaDevice &vaDevice;

  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  uint32_t vertexCount;

  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;
  uint32_t indexCount;
};
} // namespace va
