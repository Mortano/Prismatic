#pragma once
#include "peUtilDefs.h"
#include <glm/detail/type_vec3.hpp>

#pragma warning(push)
#pragma warning(disable : 4251)

namespace pe {

//! \brief Coordinate system
class PE_UTIL_API peCoordSys {
public:
  peCoordSys();
  peCoordSys(const glm::vec3 &normal, const glm::vec3 &binormal);
  peCoordSys(const glm::vec3 &normal, const glm::vec3 &binormal,
             const glm::vec3 &tangent);

  const auto &Normal() const { return _normal; }
  const auto &Binormal() const { return _binormal; }
  const auto &Tangent() const { return _tangent; }

  glm::vec3 FromWorld(const glm::vec3 &world) const;
  glm::vec3 ToWorld(const glm::vec3 &local) const;

private:
  glm::vec3 _normal, _binormal, _tangent;
};

} // namespace pe

#pragma warning(pop)
