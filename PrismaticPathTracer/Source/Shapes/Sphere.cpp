#include "Shapes\Sphere.h"
#include "Rendering/pePrimitives.h"

#include "Util\Intersections.h"

pe::Sphere::Sphere(const glm::vec3 &center, float radius)
    : center(center), radius(radius) {}

pe::Sphere::Sphere(const peSpherePrimitive &spherePrimitive) {
  center = spherePrimitive.center;
  radius = spherePrimitive.radius;
}

bool pe::Sphere::Intersects(const Ray &ray, glm::vec3 *hitPosition,
                            glm::vec3 *hitNormal,
                            peCoordSys *shadingCoordinateSystem) const {
  return RaySphereIntersection(ray, center, radius, hitPosition, hitNormal,
                               shadingCoordinateSystem);
}

pe::AABB pe::Sphere::GetBounds() const {
  return AABB{center - glm::vec3{radius, radius, radius},
              center + glm::vec3{radius, radius, radius}};
}
