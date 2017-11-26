#pragma once
#include <glm/common.hpp>

#include "Components\pePrimitiveRenderComponent.h"

namespace pe {
class AABB;
struct Ray;
struct Vertex;

bool RayTriangleIntersection(const Ray &ray, const Vertex &v0, const Vertex &v1,
                             const Vertex &v2, glm::vec3 *hitPoint,
                             glm::vec3 *hitNormal,
                             peCoordSys *shadingCoordinateSystem);

bool RaySphereIntersection(const Ray &ray, const glm::vec3 &center,
                           float radius, glm::vec3 *hitPos,
                           glm::vec3 *hitNormal,
                           peCoordSys *shadingCoordinateSystem);

bool RaySphereIntersection(const Ray &ray, const glm::vec3 &center,
                           float radius);

bool RayAABBIntersection(const Ray &ray, const AABB &aabb, glm::vec3 &hitPos1,
                         glm::vec3 &hitPos2);

} // namespace pe
