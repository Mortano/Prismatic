#pragma once
#include "Math/AABB.h"
#include <glm/detail/type_vec3.hpp>

namespace pe {
class peCoordSys;
struct Ray;
struct peSpherePrimitive;

struct Sphere {
  Sphere(const glm::vec3 &center, float radius);
  explicit Sphere(const peSpherePrimitive &spherePrimitive);

  bool Intersects(const Ray &ray, glm::vec3 *hitPosition, glm::vec3 *hitNormal,
                  peCoordSys *shadingCoordinateSystem) const;

  AABB GetBounds() const;

  glm::vec3 center;
  float radius;
};

static_assert(sizeof(Sphere) == 16, "Sphere has wrong size!");

} // namespace pe
