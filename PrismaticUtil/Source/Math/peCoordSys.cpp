#include "Math\peCoordSys.h"
#include <glm/detail/func_geometric.inl>

pe::peCoordSys::peCoordSys()
    : _normal(0, 0, 1), _binormal(0, 1, 0), _tangent(1, 0, 0) {}

pe::peCoordSys::peCoordSys(const glm::vec3 &normal, const glm::vec3 &binormal)
    : _normal(normal), _binormal(binormal),
      _tangent(glm::normalize(glm::cross(normal, binormal))) {}

pe::peCoordSys::peCoordSys(const glm::vec3 &normal, const glm::vec3 &binormal,
                           const glm::vec3 &tangent)
    : _normal(normal), _binormal(binormal), _tangent(tangent) {}

glm::vec3 pe::peCoordSys::FromWorld(const glm::vec3 &world) const {
  return {glm::dot(world, _binormal), glm::dot(world, _tangent),
          glm::dot(world, _normal)};
}

glm::vec3 pe::peCoordSys::ToWorld(const glm::vec3 &local) const {
  return {_binormal.x * local.x + _tangent.x * local.y + _normal.x * local.z,
          _binormal.y * local.x + _tangent.y * local.y + _normal.y * local.z,
          _binormal.z * local.x + _tangent.z * local.y + _normal.z * local.z};
}
