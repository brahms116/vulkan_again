
#include "va_model.hpp"
#include "va_utils.hpp"

// libs

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <vulkan/vulkan_core.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std

#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

namespace std {
template <> struct hash<va::VaModel::Vertex> {
  size_t operator()(va::VaModel::Vertex const &vertex) const {
    size_t seed = 0;
    va::hashCombine(seed, vertex.position, vertex.color, vertex.normal,
                    vertex.uv);
    return seed;
  }
};
} // namespace std

namespace va {

VaModel::VaModel(VaDevice &device, const VaModel::Builder &builder)
    : vaDevice{device} {
  createVertexBuffer(builder.vertices);
  createIndexBuffer(builder.indices);
};

std::unique_ptr<VaModel>
VaModel::createModelFromFile(VaDevice &device, const std::string &filepath) {
  Builder builder{};
  builder.loadModel(filepath);

  std::cout << "Vertex count: " << builder.vertices.size() << '\n';

  return std::make_unique<VaModel>(device, builder);
}

VaModel::~VaModel() {
  vkDestroyBuffer(vaDevice.device(), vertexBuffer, nullptr);
  vkFreeMemory(vaDevice.device(), vertexBufferMemory, nullptr);

  if (hasIndexBuffer) {
    vkDestroyBuffer(vaDevice.device(), indexBuffer, nullptr);
    vkFreeMemory(vaDevice.device(), indexBufferMemory, nullptr);
  }
};

void VaModel::bindCommandBuffer(VkCommandBuffer commandBuffer) const {
  VkBuffer buffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  if (hasIndexBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
  }
}

void VaModel::draw(VkCommandBuffer commandBuffer) const {
  if (hasIndexBuffer) {
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
  } else {
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
  }
}

std::vector<VkVertexInputBindingDescription>
VaModel::Vertex::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription>
VaModel::Vertex::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

  attributeDescriptions.push_back(
      {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});

  attributeDescriptions.push_back(
      {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});

  attributeDescriptions.push_back(
      {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});

  attributeDescriptions.push_back(
      {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

  return attributeDescriptions;
}

void VaModel::createVertexBuffer(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");

  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  vaDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(vaDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(vaDevice.device(), stagingBufferMemory);

  vaDevice.createBuffer(
      bufferSize,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

  vaDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

  vkDestroyBuffer(vaDevice.device(), stagingBuffer, nullptr);
  vkFreeMemory(vaDevice.device(), stagingBufferMemory, nullptr);
}

void VaModel::Builder::loadModel(const std::string &filepath) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                        filepath.c_str())) {
    throw std::runtime_error("Failed to load object with tinyobjloader");
  }

  vertices.clear();
  indices.clear();

  std::unordered_map<Vertex, uint32_t> map{};

  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      Vertex vertex{};
      // If index < 0, no index was provided for the face
      if (index.vertex_index >= 0) {
        vertex.position = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2],
        };

        vertex.color = {
            attrib.colors[3 * index.vertex_index + 0],
            attrib.colors[3 * index.vertex_index + 1],
            attrib.colors[3 * index.vertex_index + 2],
        };
      }

      if (index.normal_index >= 0) {
        vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2],
        };
      }

      if (index.texcoord_index >= 0) {
        vertex.uv = {
            attrib.normals[2 * index.texcoord_index + 0],
            attrib.normals[2 * index.texcoord_index + 1],
        };
      }

      if (map.count(vertex) == 0) {
        map[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }
      indices.push_back(map[vertex]);
    }
  }
}

void VaModel::createIndexBuffer(const std::vector<uint32_t> &indices) {
  indexCount = static_cast<uint32_t>(indices.size());

  if (!indexCount) {
    hasIndexBuffer = false;
    return;
  }

  hasIndexBuffer = true;
  VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  vaDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(vaDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(vaDevice.device(), stagingBufferMemory);

  vaDevice.createBuffer(
      bufferSize,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

  vaDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

  vkDestroyBuffer(vaDevice.device(), stagingBuffer, nullptr);
  vkFreeMemory(vaDevice.device(), stagingBufferMemory, nullptr);
}
} // namespace va
