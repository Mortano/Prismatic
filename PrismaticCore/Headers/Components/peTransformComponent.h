#pragma once
#include "Entities\Entity.h"

#include <glm\matrix.hpp>

#pragma warning(push)
#pragma warning(disable : 4251)

namespace pe {

//! \brief Component that stores transformation data
struct PE_CORE_API peTransformComponent : peComponent<peTransformComponent> {
  glm::mat4 transformation;
};

} // namespace pe

#pragma warning(pop)
