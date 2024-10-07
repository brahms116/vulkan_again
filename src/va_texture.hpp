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

  VkDescriptorImageInfo descriptorInfo() const;

  VaTexture(VaDevice &vaDevice, const CreateImageProperties &properties);
  VaTexture(VaDevice &vaDevice, const std::string &filePath);

private:
  VaDevice &vaDevice;
  VkImage image;
  VkDeviceMemory imageMemory;
  VkImageView imageView;
  VkSampler sampler;

  void initializeImage(const CreateImageProperties &properties);
  void initializeImageFromFile(const std::string &filePath);
};
} // namespace va
