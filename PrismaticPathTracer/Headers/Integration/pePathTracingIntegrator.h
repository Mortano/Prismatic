#pragma once
#include "DataStructures/peVector.h"
#include "Rendering/Utility/peBxDF.h"
#include "Sampling/peLightSampler.h"
#include "Sampling/peSampler.h"
#include "peIntegrator.h"

namespace pe {
struct SceneHit;
class peScene;

//! \brief Integrator that uses path tracing
class pePathTracingIntegrator : public peSurfaceIntegrator {
public:
  explicit pePathTracingIntegrator(uint32_t maxDepth);

  Spectrum_t Estimate(const peScene &scene, const Sample &sample,
                      const SceneHit &hit, const glm::vec3 &wo,
                      const BSDF &bsdf, BxDFType flags,
                      std::default_random_engine rng) override;

  //! \brief Allocates the samples required for the integrator
  void AllocateSamples(Sample &sample, const peScene &scene) override;

private:
  //! \brief The path depth to which sampling is done with weighted random
  //! numbers, after this
  //!        uniform random numbers are used
  constexpr static uint32_t SampleDepth = 3;

  const uint32_t _maxDepth;
  const uint32_t _numSamplesPerPixel;
  peVector<LightSampleOffset> _lightSampleOffsets;
  peVector<BSDFSampleOffset> _bsdfSampleOffsets;
  peVector<BSDFSampleOffset> _pathSampleOffsets;
  peVector<uint32_t> _sampleOffsets;
};

} // namespace pe
