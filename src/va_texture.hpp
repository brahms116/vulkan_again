#pragma once

#include "va_buffer.hpp"
#include "va_engine_device.hpp"

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

  static VaTexture fromFilePath(VaDevice &vaDevice,
                                const std::string &filePath);

  static VaTexture
  fromCreateImageProperties(VaDevice &vaDevice,
                            const CreateImageProperties &properties);

  void transitionImageLayout(VkFormat format, VkImageLayout oldLayout,
                             VkImageLayout newLayout);

private:
  VaTexture(VaDevice &device, VkImage image, VkDeviceMemory memory,
            VkImageView imageView);

  VaDevice &vaDevice;
  VkImage image;
  VkDeviceMemory imageMemory;
  VkImageView imageView;
};
} // namespace va
