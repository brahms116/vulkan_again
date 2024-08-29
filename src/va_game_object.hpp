#pragma once

#include "va_model.hpp"

#include <memory>

namespace va {

struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.f, 1.f, 1.f};

  /** Y X Z extrinsic rotation */
  glm::vec3 rotation{0.f, 0.f, 0.f};

  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4() const {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    return glm::mat4{{
                         scale.x * (c1 * c3 + s1 * s2 * s3),
                         scale.x * (c2 * s3),
                         scale.x * (c1 * s2 * s3 - c3 * s1),
                         0.0f,
                     },
                     {
                         scale.y * (c3 * s1 * s2 - c1 * s3),
                         scale.y * (c2 * c3),
                         scale.y * (c1 * c3 * s2 + s1 * s3),
                         0.0f,
                     },
                     {
                         scale.z * (c2 * s1),
                         scale.z * (-s2),
                         scale.z * (c1 * c2),
                         0.0f,
                     },
                     {translation.x, translation.y, translation.z, 1.0f}};
  }
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
