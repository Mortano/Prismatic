#pragma once
#include "Rendering/Utility/peBxDF.h"
#include <glm/detail/type_vec3.hpp>

namespace pe {
struct Sample;
struct SceneHit;
class BSDF;
enum class BxDFType;
class peScene;

//! \brief Base class for all surface integrators
struct peSurfaceIntegrator {
  virtual ~peSurfaceIntegrator() = default;

  //! \brief Estimate the incident light in the direction 'wo' from the hit
  //! position
  virtual Spectrum_t Estimate(const peScene &scene, const Sample &sample,
                              const SceneHit &hit, const glm::vec3 &wo,
                              const BSDF &bsdf, BxDFType flags,
                              std::default_random_engine rng) = 0;

  //! \brief Allocates the samples required for the integrator
  virtual void AllocateSamples(Sample &sample, const peScene &scene) = 0;
};

} // namespace pe
