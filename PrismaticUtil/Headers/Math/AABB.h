#pragma once

#include <algorithm>
#include <glm\common.hpp>
#include <limits>

#include "Math\MathUtil.h"
#include "peUtilDefs.h"

#pragma warning(push)
#pragma warning(disable : 4251)

namespace pe {

// Taken directly from PBRT2 on GitHub

class PE_UTIL_API AABB {
public:
  // AABB Public Methods
  AABB() {
    constexpr auto inf = (std::numeric_limits<float>::max)();
    constexpr auto negInf = std::numeric_limits<float>::lowest();
    _pMin = glm::vec3(inf, inf, inf);
    _pMax = glm::vec3(negInf, negInf, negInf);
  }
  explicit AABB(const glm::vec3 &p) : _pMin(p), _pMax(p) {}
  AABB(const glm::vec3 &p1, const glm::vec3 &p2) {
    _pMin = glm::vec3{(std::min)(p1.x, p2.x), (std::min)(p1.y, p2.y),
                      (std::min)(p1.z, p2.z)};
    _pMax = glm::vec3{(std::max)(p1.x, p2.x), (std::max)(p1.y, p2.y),
                      (std::max)(p1.z, p2.z)};
  }
  friend AABB PE_UTIL_API Union(const AABB &b, const glm::vec3 &p);
  friend AABB PE_UTIL_API Union(const AABB &b, const AABB &b2);
  bool Overlaps(const AABB &b) const {
    bool x = (_pMax.x >= b._pMin.x) && (_pMin.x <= b._pMax.x);
    bool y = (_pMax.y >= b._pMin.y) && (_pMin.y <= b._pMax.y);
    bool z = (_pMax.z >= b._pMin.z) && (_pMin.z <= b._pMax.z);
    return (x && y && z);
  }
  bool Inside(const glm::vec3 &pt) const {
    return (pt.x >= _pMin.x && pt.x <= _pMax.x && pt.y >= _pMin.y &&
            pt.y <= _pMax.y && pt.z >= _pMin.z && pt.z <= _pMax.z);
  }
  void Expand(float delta) {
    _pMin -= glm::vec3(delta, delta, delta);
    _pMax += glm::vec3(delta, delta, delta);
  }
  float SurfaceArea() const {
    auto d = _pMax - _pMin;
    return 2.f * (d.x * d.y + d.x * d.z + d.y * d.z);
  }
  float Volume() const {
    auto d = _pMax - _pMin;
    return d.x * d.y * d.z;
  }
  int MaximumExtent() const {
    auto diag = _pMax - _pMin;
    if (diag.x > diag.y && diag.x > diag.z)
      return 0;
    else if (diag.y > diag.z)
      return 1;
    else
      return 2;
  }
  const glm::vec3 &operator[](int i) const;
  glm::vec3 &operator[](int i);
  glm::vec3 Lerp(float tx, float ty, float tz) const {
    return glm::vec3(pe::Lerp(tx, _pMin.x, _pMax.x),
                     pe::Lerp(ty, _pMin.y, _pMax.y),
                     pe::Lerp(tz, _pMin.z, _pMax.z));
  }
  glm::vec3 Offset(const glm::vec3 &p) const {
    return {(p.x - _pMin.x) / (_pMax.x - _pMin.x),
            (p.y - _pMin.y) / (_pMax.y - _pMin.y),
            (p.z - _pMin.z) / (_pMax.z - _pMin.z)};
  }
  void BoundingSphere(glm::vec3 &c, float &rad) const;

  bool operator==(const AABB &b) const {
    return b._pMin == _pMin && b._pMax == _pMax;
  }
  bool operator!=(const AABB &b) const {
    return b._pMin != _pMin || b._pMax != _pMax;
  }

  const auto &Min() const { return _pMin; }
  const auto &Max() const { return _pMax; }

private:
  glm::vec3 _pMin, _pMax;
};

} // namespace pe

#pragma warning(pop)
