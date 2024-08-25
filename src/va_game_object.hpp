#pragma once

#include "va_model.hpp"

#include <memory>

namespace va {

struct Transform2dComponent {
  glm::vec2 translation{};
  glm::vec2 scale{1.f, 1.f};
  float rotation;
  glm::mat2 mat2() {
    const float s = glm::sin(rotation);
    const float c = glm::cos(rotation);

    glm::mat2 rotMatrix = {{c, s}, {-s, c}};
    glm::mat2 scaleMat{{scale.x, 0.f}, {0.f, scale.y}};
    return rotMatrix * scaleMat;
  };
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
  glm::vec4 color{};

private:
  id_t id;
  VaGameObject(id_t objId) : id{objId} {}
};
} // namespace va
