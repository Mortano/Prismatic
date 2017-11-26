#pragma once

#include <glm\common.hpp>

namespace pe {

//! \brief Analytical sphere primitive
struct peSpherePrimitive {
  glm::vec3 center;
  float radius;
};

} // namespace pe
