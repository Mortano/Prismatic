#include "Components\peCameraComponent.h"

void pe::peCameraComponentSystem::SetPerspectiveProjection(
    const Radians fov, const float aspect, const float zNear, const float zFar,
    peComponentHandle<peCameraComponent> component) const {
  if (!component)
    throw std::runtime_error{"Component must not be null!"};

  const auto tanAngle = std::tan(fov);

  auto &projMat = component->projection;
  projMat[0][0] = 1 / tanAngle * aspect;
  projMat[0][1] = 0;
  projMat[0][2] = 0;
  projMat[0][3] = 0;

  projMat[1][0] = 1 / tanAngle;
  projMat[1][1] = 0;
  projMat[1][2] = 0;
  projMat[1][3] = 0;

  projMat[2][0] = zFar / (zNear - zFar);
  projMat[2][1] = -1;
  projMat[2][2] = 0;
  projMat[2][3] = 0;

  projMat[3][0] = (zFar * zNear) / (zNear - zFar);
  projMat[3][1] = 0;
  projMat[3][2] = 0;
  projMat[3][3] = 0;

  auto &comp = *component;
  comp.fov = fov;
  comp.zMin = zNear;
  comp.zMax = zFar;
}

void pe::peCameraComponentSystem::LookAt(
    const glm::vec3 &at, const glm::vec3 &to, const glm::vec3 &up,
    peComponentHandle<peCameraComponent> component) const {
  if (!component)
    throw std::runtime_error{"Component must not be null!"};

  auto z = glm::normalize(to - at);
  const auto x = glm::cross(z, glm::normalize(up));
  const auto y = glm::cross(x, z);

  auto &viewMat = component->view;

  // X
  viewMat[0][0] = x.x;
  viewMat[0][1] = x.y;
  viewMat[0][2] = x.z;
  viewMat[0][3] = 0;

  viewMat[1][0] = y.x;
  viewMat[1][1] = y.y;
  viewMat[1][2] = y.z;
  viewMat[1][3] = 0;

  viewMat[2][0] = z.x;
  viewMat[2][1] = z.y;
  viewMat[2][2] = z.z;
  viewMat[2][3] = 0;

  viewMat[3][0] = at.x;
  viewMat[3][1] = at.y;
  viewMat[3][2] = at.z;
  viewMat[3][3] = 1;
}
