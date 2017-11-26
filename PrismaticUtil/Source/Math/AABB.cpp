#include "Math\AABB.h"

namespace pe {

AABB Union(const AABB &b, const glm::vec3 &p) {
  AABB ret = b;
  ret._pMin.x = std::min(b._pMin.x, p.x);
  ret._pMin.y = std::min(b._pMin.y, p.y);
  ret._pMin.z = std::min(b._pMin.z, p.z);
  ret._pMax.x = std::max(b._pMax.x, p.x);
  ret._pMax.y = std::max(b._pMax.y, p.y);
  ret._pMax.z = std::max(b._pMax.z, p.z);
  return ret;
}

AABB Union(const AABB &b, const AABB &b2) {
  AABB ret;
  ret._pMin.x = std::min(b._pMin.x, b2._pMin.x);
  ret._pMin.y = std::min(b._pMin.y, b2._pMin.y);
  ret._pMin.z = std::min(b._pMin.z, b2._pMin.z);
  ret._pMax.x = std::max(b._pMax.x, b2._pMax.x);
  ret._pMax.y = std::max(b._pMax.y, b2._pMax.y);
  ret._pMax.z = std::max(b._pMax.z, b2._pMax.z);
  return ret;
}

void AABB::BoundingSphere(glm::vec3 &c, float &rad) const {
  c = .5f * _pMin + .5f * _pMax;
  rad = Inside(c) ? glm::distance(c, _pMax) : 0.f;
}

} // namespace pe
