#pragma once

#include "va_engine_device.hpp"
#include <memory>

namespace va {
class VaTexture {
public:
  ~VaTexture();

  struct CreateImageProperties {
    uint32_t width;
    uint32_t height;
    VkFormat format;
    VkImageTiling tiling;
    VkImageUsageFlags imageUsage;
    VkMemoryPropertyFlags memoryProperties;
  };

  VaTexture(const VaTexture &) = delete;
  VaTexture &operator=(const VaTexture &) = delete;

  VaTexture(VaTexture &&other) = default;

  static std::unique_ptr<VaTexture> fromFilePath(VaDevice &vaDevice,
                                                 const std::string &filePath);

  static std::unique_ptr<VaTexture>
  fromCreateImageProperties(VaDevice &vaDevice,
                            const CreateImageProperties &properties);

  void transitionImageLayout(VkFormat format, VkImageLayout oldLayout,
                             VkImageLayout newLayout);

  VaTexture(VaDevice &device, VkImage image, VkDeviceMemory memory,
            VkImageView imageView, VkSampler sampler);

private:
  VaDevice &vaDevice;
  VkImage image;
  VkDeviceMemory imageMemory;
  VkImageView imageView;
  VkSampler sampler;
};
} // namespace va
