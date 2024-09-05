#pragma once

#include "va_model.hpp"

#include <memory>

namespace va {

struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.f, 1.f, 1.f};

  /** Y X Z extrinsic rotation */
  glm::vec3 rotation{0.f, 0.f, 0.f};

  glm::mat4 mat4() const;
  glm::mat4 normalMatrix() const;
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

  std::shared_ptr<const VaModel> model{};

  TransformComponent transform{};
  glm::vec4 color{};

private:
  id_t id;
  VaGameObject(id_t objId) : id{objId} {}
};
} // namespace va
