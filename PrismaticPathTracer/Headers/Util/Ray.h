#pragma once

#include <glm\common.hpp>
#include <span.h>

namespace pe {
struct Sample;
struct peCameraComponent;

//! \brief Ray structure
struct Ray {
  Ray();
  Ray(const glm::vec3 &origin, const glm::vec3 &dir, float t);

  glm::vec3 origin, direction;
  mutable float t;
};

//! \brief Computes camera rays from the given set of samples
void GetPrimaryRaysFromSamples(gsl::span<Ray> rays, gsl::span<Sample> samples,
                               const peCameraComponent &camera,
                               const glm::uvec2 &screenSize);

} // namespace pe
