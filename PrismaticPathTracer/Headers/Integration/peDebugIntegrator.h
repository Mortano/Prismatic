#pragma once
#include "peIntegrator.h"

namespace pe {

enum class DebugVisualizationMode {
  GeometryNormals,
  ShadingNormal,
  ShadingBinormal,
  ShadingTangent,
  Sample
};

//! \brief Integrator used for debugging visualization
struct peDebugIntegrator : peSurfaceIntegrator {
  peDebugIntegrator();

  Spectrum_t Estimate(const peScene &scene, const Sample &sample,
                      const SceneHit &hit, const glm::vec3 &wo,
                      const BSDF &bsdf, BxDFType flags,
                      std::default_random_engine rng) override;
  void AllocateSamples(Sample &sample, const peScene &scene) override;

  void SetVisualizationMode(DebugVisualizationMode mode);

private:
  DebugVisualizationMode _mode;
};

} // namespace pe
