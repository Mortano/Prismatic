#pragma once
#include "Rendering/Utility/peBxDF.h"

#include "Sampling/peLightSampler.h"
#include "Sampling/peSampler.h"
#include "peIntegrator.h"
#include <random>

namespace pe {
struct SceneHit;
class peScene;

//! \brief Integrator that solves the direct lighting integral
class peDirectLightIntegrator : public peSurfaceIntegrator {
public:
  Spectrum_t Estimate(const peScene &scene, const Sample &sample,
                      const SceneHit &hit, const glm::vec3 &wo,
                      const BSDF &bsdf, BxDFType flags,
                      std::default_random_engine rng) override;

  //! \brief Allocates the samples required for the integrator
  void AllocateSamples(Sample &sample, const peScene &scene) override;

private:
  peVector<LightSampleOffset> _lightSampleOffsets;
  peVector<BSDFSampleOffset> _bsdfSampleOffsets;
};

} // namespace pe
