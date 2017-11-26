#pragma once
#include "Entities\Entity.h"
#include "Type\peColor.h"

#include <glm\common.hpp>

#pragma warning(push)
#pragma warning(disable : 4251)

namespace pe {

//! \brief Component for a point light source
struct PE_CORE_API pePointLightComponent : peComponent<pePointLightComponent> {
  glm::vec3 position;
  RGB_32BitFloat intensity;
  uint32_t numSamples;
};

//! \brief Component for a directional light source
struct PE_CORE_API peDirectionalLightComponent
    : peComponent<peDirectionalLightComponent> {
  glm::vec3 orientation;
  RGB_32BitFloat intensity;
  uint32_t numSamples;
};

} // namespace pe

#pragma warning(pop)
