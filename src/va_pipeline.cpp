#include "va_pipeline.hpp"

#include <fstream>
#include <stdexcept>

#include <iostream>

namespace va {

VaPipeline::VaPipeline(const std::string &vertexShaderFilePath,
                       const std::string &fragmentShaderFilePath) {
  createGraphicsPipeline(vertexShaderFilePath, fragmentShaderFilePath);
}

std::vector<char> VaPipeline::readFile(const std::string &filePath) {

  std::ifstream file{filePath, std::ios::ate | std::ios::binary};

  if (!file.is_open()) {
    throw std::runtime_error("fialed to open file: " + filePath);
  }

  size_t fileSize = static_cast<size_t>(file.tellg());

  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();
  return buffer;
};

void VaPipeline::createGraphicsPipeline(
    const std::string &vertexShaderFilePath,
    const std::string &fragmentShaderFilePath) {
  auto vertCode = readFile(vertexShaderFilePath);
  auto fragCode = readFile(fragmentShaderFilePath);

  std::cout << "Vertex code size: " << vertCode.size() << '\n';
  std::cout << "Frag code size: " << fragCode.size() << '\n';
}
} // namespace va
