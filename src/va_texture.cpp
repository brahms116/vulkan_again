#include "va_texture.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace va {
VaTexture::VaTexture(VaDevice &vaDevice, VkImage image, VkDeviceMemory memory,
                     VkImageView imageView)
    : vaDevice(vaDevice), image(image), imageMemory(memory),
      imageView(imageView) {}

VaTexture::~VaTexture() {
  vkDestroyImageView(vaDevice.device(), imageView, nullptr);
  vkDestroyImage(vaDevice.device(), image, nullptr);
  vkFreeMemory(vaDevice.device(), imageMemory, nullptr);
};

void VaTexture::transitionImageLayout(VkFormat format, VkImageLayout oldLayout,
                                      VkImageLayout newLayout) {

  VkImageMemoryBarrier barrier;
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }

  VkCommandBuffer commandBuffer = vaDevice.beginSingleTimeCommands();

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);

  vaDevice.endSingleTimeCommands(commandBuffer);
}

VaTexture
VaTexture::fromCreateImageProperties(VaDevice &vaDevice,
                                     const CreateImageProperties &properties) {

  VkImage image;
  VkDeviceMemory imageMemory;

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = properties.width;
  imageInfo.extent.height = properties.height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = properties.format;
  imageInfo.tiling = properties.tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = properties.imageUsage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  vaDevice.createImageWithInfo(imageInfo, properties.memoryProperties, image,
                               imageMemory);

  VkImageView imageView;
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;

  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(vaDevice.device(), &viewInfo, nullptr, &imageView) !=
      VK_SUCCESS) {
    throw std::runtime_error("Could not create image view");
  }

  return VaTexture(vaDevice, image, imageMemory, imageView);
}

VaTexture VaTexture::fromFilePath(VaDevice &device,
                                  const std::string &filePath) {
  int texWidth, texHeight, texChannels;

  stbi_uc *pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight,
                              &texChannels, STBI_rgb_alpha);

  if (!pixels) {
    throw std::runtime_error("failed to load texture image: " + filePath);
  }

  VaTexture::CreateImageProperties createImageProperties{};
  createImageProperties.width = texWidth;
  createImageProperties.height = texHeight;
  createImageProperties.format = VK_FORMAT_R8G8B8A8_SRGB;
  createImageProperties.tiling = VK_IMAGE_TILING_OPTIMAL;
  createImageProperties.imageUsage =
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  createImageProperties.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  VaTexture texture =
      VaTexture::fromCreateImageProperties(device, createImageProperties);

  texture.transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB,
                                VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // Copy to buffer

  VkDeviceSize imageSize = texWidth * texHeight * 4;
  VaBuffer stagingBuffer =
      VaBuffer(device, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)pixels);

  device.copyBufferToImage(stagingBuffer.getBuffer(), texture.image, texWidth,
                           texHeight, 1);

  texture.transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  return texture;
}
} // namespace va
