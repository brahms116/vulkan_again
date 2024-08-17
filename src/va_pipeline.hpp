#pragma once

#include <string>
#include <vector>

namespace va {
class VaPipeline {
public:
  VaPipeline(const std::string &vertexShaderFilePath,
             const std::string &fragmentShaderFilePath);

private:
  static std::vector<char> readFile(const std::string &filePath);

  void createGraphicsPipeline(const std::string &vertexShaderFilePath,
                              const std::string &fragmentShaderFilePath);
};
} // namespace va
