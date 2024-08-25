#pragma once

#include "va_model.hpp"

#include <memory>

namespace va {

struct Transform2dComponent {
  glm::vec2 translation{};
  const glm::mat2 mat2() { return glm::mat2{1.f}; };
};

class VaGameObject {
public:
  using id_t = int;

  VaGameObject(const VaGameObject &) = delete;
  void operator=(const VaGameObject &) = delete;

  VaGameObject(VaGameObject &&) = default;
  VaGameObject &operator=(VaGameObject &&) = default;

  static VaGameObject create() {
    static id_t currentId = 0;
    return VaGameObject(currentId++);
  }

  const id_t getId() { return id; };

  std::shared_ptr<VaModel> model{};

  Transform2dComponent transform2d{};
  glm::vec3 color{};

private:
  id_t id;
  VaGameObject(id_t objId) : id{objId} {}
};
} // namespace va
