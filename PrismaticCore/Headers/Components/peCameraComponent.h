#pragma once

#include "Entities\Entity.h"
#include "Type\peUnits.h"

#include <glm\mat4x4.hpp>

#pragma warning(push)
#pragma warning(disable : 4251)

namespace pe {

struct peCameraComponent;

//! \brief System that handles camera components
struct PE_CORE_API peCameraComponentSystem
    : peComponentSystem<peCameraComponentSystem> {
  using Component_t = peCameraComponent;

  //! \brief Sets the given perspective projection to the given camera
  //! \param fov Field of view
  //! \param aspect Aspect ratio
  //! \param zNear Near clipping plane
  //! \param zFar Far clipping plane
  //! \param component Camera component handle
  void SetPerspectiveProjection(
      const Radians fov, const float aspect, const float zNear,
      const float zFar, peComponentHandle<peCameraComponent> component) const;

  //! \brief Makes the given camera look at the given point
  //! \param at The new position of the camera
  //! \param to The point to which the camera should look
  //! \param up The up vector for the camera
  //! \param component Camera component handle
  void LookAt(const glm::vec3 &at, const glm::vec3 &to, const glm::vec3 &up,
              peComponentHandle<peCameraComponent> component) const;
};

//! \brief Component for cameras
struct PE_CORE_API peCameraComponent : peComponent<peCameraComponent> {
  using System_t = peCameraComponentSystem;

  glm::mat4 view;
  glm::mat4 projection;
  Radians fov;
  float zMin, zMax;
};

} // namespace pe

#pragma warning(pop)
