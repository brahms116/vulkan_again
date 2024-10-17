#include "va_texture.hpp"
#include "va_buffer.hpp"

#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace va {

VaTexture::VaTexture(VaDevice &vaDevice,
                     const CreateImageProperties &properties)
    : vaDevice{vaDevice} {
  initializeImage(properties);
}

VaTexture::VaTexture(VaDevice &vaDevice, const std::string &filePath)
    : vaDevice{vaDevice} {
  initializeImageFromFile(filePath);
}

VaTexture::~VaTexture() {
  vkDestroySampler(vaDevice.device(), sampler, nullptr);
  vkDestroyImageView(vaDevice.device(), imageView, nullptr);
  vkDestroyImage(vaDevice.device(), image, nullptr);
  vkFreeMemory(vaDevice.device(), imageMemory, nullptr);
};

void VaTexture::transitionImageLayout(VkFormat format, VkImageLayout oldLayout,
                                      VkImageLayout newLayout) {

  VkImageMemoryBarrier barrier{};
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

void VaTexture::initializeImageFromFile(const std::string &filePath) {
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
  createImageProperties.imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

  initializeImage(createImageProperties);

  // Copy to buffer

  transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkDeviceSize imageSize = texWidth * texHeight * 4;
  VaBuffer stagingBuffer =
      VaBuffer(vaDevice, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)pixels);

  vaDevice.copyBufferToImage(stagingBuffer.getBuffer(), image, texWidth,
                             texHeight, 1);

  transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VaTexture::initializeImage(const CreateImageProperties &properties) {
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

  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = properties.format;

  viewInfo.subresourceRange.aspectMask = properties.imageAspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(vaDevice.device(), &viewInfo, nullptr, &imageView) !=
      VK_SUCCESS) {
    throw std::runtime_error("Could not create image view");
  }

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;

  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = true;

  samplerInfo.maxAnisotropy = vaDevice.properties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

  samplerInfo.unnormalizedCoordinates = VK_FALSE;

  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  if (vkCreateSampler(vaDevice.device(), &samplerInfo, nullptr, &sampler) !=
      VK_SUCCESS) {
    throw std::runtime_error("Could not create image sampler");
  }
}

VkDescriptorImageInfo VaTexture::descriptorInfo() const {
  VkDescriptorImageInfo info{};
  info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  info.imageView = imageView;
  info.sampler = sampler;
  return info;
}

} // namespace va
