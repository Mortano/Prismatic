#include "Util\Intersections.h"
#include "Util/Ray.h"

#include "Math/AABB.h"
#include "Math\peCoordSys.h"
#include "Shapes\Triangle.h"
#include <limits>

bool pe::RayTriangleIntersection(const Ray &ray, const Vertex &v0,
                                 const Vertex &v1, const Vertex &v2,
                                 glm::vec3 *hitPoint, glm::vec3 *hitNormal,
                                 peCoordSys *shadingCoordinateSystem) {
  const auto edge1 = v1.position - v0.position;
  const auto edge2 = v2.position - v0.position;
  const auto pvec = glm::cross(ray.direction, edge2);

  const auto det = glm::dot(edge1, pvec);
  if (std::abs(det) <= std::numeric_limits<float>::epsilon())
    return false;
  const auto invDet = 1 / det;
  const auto tvec = ray.origin - v0.position;
  const auto u = glm::dot(tvec, pvec) * invDet;
  if (u < 0 || u > 1)
    return false;
  const auto qvec = glm::cross(tvec, edge1);
  const auto v = glm::dot(ray.direction, qvec) * invDet;
  if (v < 0 || u + v > 1)
    return false;
  const auto t = glm::dot(edge2, qvec) * invDet;
  if (t >= 0 && t < ray.t) {
    ray.t = t;
    if (hitPoint)
      *hitPoint = v0.position * (1 - u - v) + v1.position * u + v2.position * v;
    if (hitNormal)
      *hitNormal = glm::normalize(glm::cross(edge1, edge2));
    if (shadingCoordinateSystem) {
      // auto shadingNormal =
      //    v0.normal * (1 - u - v) + v1.normal * u + v2.normal * v;
      // TODO If triangle has parametric coordinates, use them here
      const auto du1 = -1.f; //= uvs[0][0] - uvs[2][0];
      const auto du2 = 0.f;  //= uvs[1][0] - uvs[2][0];
      const auto dv1 = -1.f; //= uvs[0][1] - uvs[2][1];
      const auto dv2 = -1.f; //= uvs[1][1] - uvs[2][1];
      auto dp1 = v0.position - v2.position, dp2 = v1.position - v2.position;
      const auto determinant = du1 * dv2 - dv1 * du2;
      // if (determinant == 0.f) {
      //  // Handle zero determinant for triangle partial derivative matrix
      //  CoordinateSystem(Normalize(Cross(e2, e1)), &dpdu, &dpdv);
      //} else {
      const auto invdet = 1.f / determinant;
      const auto dpdu = glm::normalize((dv2 * dp1 - dv1 * dp2) * invdet);
      const auto dpdv = glm::normalize((-du2 * dp1 + du1 * dp2) * invdet);
      // auto normal = glm::normalize(glm::cross(dpdu, dpdv));
      auto normal = glm::normalize(v0.normal * (1 - u - v) + v1.normal * u +
                                   v2.normal * v);
      // auto tangent = dpdu;
      auto tangent = glm::normalize(dp1);
      auto binormal = glm::normalize(glm::cross(normal, tangent));
      *shadingCoordinateSystem = {normal, tangent, binormal};
    }

    return true;
  }
  return false;
}

bool pe::RaySphereIntersection(const Ray &ray, const glm::vec3 &center,
                               const float radius, glm::vec3 *hitPos,
                               glm::vec3 *hitNormal,
                               peCoordSys *shadingCoordinateSystem) {
  const auto rSqr = radius * radius;
  const auto co = center - ray.origin;

  const auto tca = glm::dot(co, ray.direction);
  const auto coSqr = glm::dot(co, co);
  const auto dSqr = coSqr - tca * tca;

  if (dSqr > rSqr)
    return false;

  const auto thc = sqrtf(rSqr - coSqr + tca * tca);
  const auto t0 = tca - thc;
  const auto t1 = tca + thc;

  if (t0 >= ray.t)
    return false;              // Not close enough
  if (t0 < 0) {                // First intersection is behind ray origin
    if (t1 < 0 || t1 >= ray.t) // Second intersection is also behind ray origin,
                               // or behind closer intersection
      return false;
    // t1 is a hit point!
    ray.t = t1;
    if (hitPos)
      *hitPos = ray.origin + (ray.direction * t1);
    if (hitNormal)
      *hitNormal = glm::normalize((ray.origin + (ray.direction * t1)) - center);
    return true;
  }
  // t0 is a hit point
  ray.t = t0;
  if (hitPos)
    *hitPos = ray.origin + (ray.direction * t0);
  if (hitNormal)
    *hitNormal = glm::normalize((ray.origin + (ray.direction * t0)) - center);
  return true;
}

bool pe::RaySphereIntersection(const Ray &ray, const glm::vec3 &center,
                               float radius) {
  const auto rSqr = radius * radius;
  const auto co = center - ray.origin;

  const auto tca = glm::dot(co, ray.direction);
  const auto coSqr = glm::dot(co, co);
  const auto dSqr = coSqr - tca * tca;

  if (dSqr > rSqr)
    return false;

  const auto thc = sqrtf(rSqr - coSqr + tca * tca);
  const auto t0 = tca - thc;
  const auto t1 = tca + thc;

  if (t0 >= ray.t)
    return false;              // Not close enough
  if (t0 < 0) {                // First intersection is behind ray origin
    if (t1 < 0 || t1 >= ray.t) // Second intersection is also behind ray origin,
                               // or behind closer intersection
      return false;
    return true;
  }
  return true;
}

bool pe::RayAABBIntersection(const Ray &ray, const AABB &aabb,
                             glm::vec3 &hitPos1, glm::vec3 &hitPos2) {
  float t0 = 0.f, t1 = ray.t;
  for (int i = 0; i < 3; ++i) {
    // Update interval for _i_th bounding box slab
    float invRayDir = 1.f / ray.direction[i];
    float tNear = (aabb.Min()[i] - ray.origin[i]) * invRayDir;
    float tFar = (aabb.Max()[i] - ray.origin[i]) * invRayDir;

    // Update parametric interval from slab intersection $t$s
    if (tNear > tFar)
      std::swap(tNear, tFar);
    t0 = tNear > t0 ? tNear : t0;
    t1 = tFar < t1 ? tFar : t1;
    if (t0 > t1)
      return false;
  }
  hitPos1 = ray.origin + ray.direction * t0;
  hitPos2 = ray.origin + ray.direction * t1;
  return true;
}
