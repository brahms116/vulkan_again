#include "keyboard_movement_controller.hpp"
#include <cmath>
#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>
#include <limits>

#define isKeyDown(key) glfwGetKey(window, key) == GLFW_PRESS

namespace va {

void KeyboardMovementController::updateTransformXZ(GLFWwindow *window,
                                                   VaGameObject &gameObject,
                                                   float deltaTime) {

  glm::vec3 rotation = glm::vec3(0.f);
  if (isKeyDown(keys.lookLeft))
    rotation.y -= 1.f;
  if (isKeyDown(keys.lookRight))
    rotation.y += 1.f;

  if (isKeyDown(keys.lookUp))
    rotation.x += 1.f;
  if (isKeyDown(keys.lookDown))
    rotation.x -= 1.f;

  if (glm::dot(rotation, rotation) > std::numeric_limits<float>::epsilon()) {
    gameObject.transform.rotation +=
        deltaTime * turnSpeed * glm::normalize(rotation);
  }

  gameObject.transform.rotation.x =
      glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);

  gameObject.transform.rotation.y =
      glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

  const float yaw = gameObject.transform.rotation.y;
  const glm::vec3 forward(sin(yaw), 0.f, cos(yaw));
  const glm::vec3 right(forward.z, 0.f, -forward.x);
  const glm::vec3 up(0.f, -1.f, 0.f);
  glm::vec3 movement{0.f};
  if (isKeyDown(keys.moveUp))
    movement += up;
  if (isKeyDown(keys.moveDown))
    movement -= up;
  if (isKeyDown(keys.moveLeft))
    movement -= right;
  if (isKeyDown(keys.moveRight))
    movement += right;
  if (isKeyDown(keys.moveForward))
    movement += forward;
  if (isKeyDown(keys.moveBackward))
    movement -= forward;

  if (glm::dot(movement, movement) > std::numeric_limits<float>::epsilon()) {
    gameObject.transform.translation +=
        deltaTime * moveSpeed * glm::normalize(movement);
  }
}

} // namespace va
