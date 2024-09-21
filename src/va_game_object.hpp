#pragma once

#include "va_model.hpp"

#include <memory>
#include <unordered_map>

namespace va {

struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.f, 1.f, 1.f};

  /** Y X Z extrinsic rotation */
  glm::vec3 rotation{0.f, 0.f, 0.f};

  glm::mat4 mat4() const;
  glm::mat4 normalMatrix() const;
};

struct PointLightComponent {
  float lightIntesity{1.f};
  glm::vec4 color{1.f, 1.f, 1.f, 1.f};
};

class VaGameObject {
public:
  using id_t = int;
  using Map = std::unordered_map<id_t, VaGameObject>;

  VaGameObject(const VaGameObject &) = delete;
  void operator=(const VaGameObject &) = delete;

  VaGameObject(VaGameObject &&) = default;
  VaGameObject &operator=(VaGameObject &&) = default;

  static VaGameObject create() {
    static id_t currentId = 0;
    return VaGameObject(currentId++);
  }

  static VaGameObject makePointLight(float intensity = 10.f, float radius = 0.f,
                                     glm::vec4 color = glm::vec4(1.f));

  const id_t getId() { return id; };

  TransformComponent transform{};

  std::shared_ptr<const VaModel> model{};
  std::unique_ptr<const PointLightComponent> pointLightComponent = nullptr;

private:
  id_t id;
  VaGameObject(id_t objId) : id{objId} {}
};
} // namespace va
