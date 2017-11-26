#include "Integration\peDebugIntegrator.h"
#include "Sampling/peSampler.h"
#include "Scene/peScene.h"

static pe::Spectrum_t NormalToSpectrum(const glm::vec3 &normal) {
  return pe::Spectrum_t{0.5f + normal.x / 2, 0.5f + normal.y / 2,
                        0.5f + normal.z / 2};
}

pe::peDebugIntegrator::peDebugIntegrator()
    : _mode(DebugVisualizationMode::GeometryNormals) {}

pe::Spectrum_t pe::peDebugIntegrator::Estimate(const peScene &scene,
                                               const Sample &sample,
                                               const SceneHit &hit,
                                               const glm::vec3 &wo,
                                               const BSDF &bsdf, BxDFType flags,
                                               std::default_random_engine rng) {
  switch (_mode) {
  case pe::DebugVisualizationMode::GeometryNormals:
    return NormalToSpectrum(hit.hitNormal);
  case pe::DebugVisualizationMode::ShadingNormal:
    return NormalToSpectrum(hit.shadingCoordinateSystem.Normal());
  case pe::DebugVisualizationMode::ShadingBinormal:
    return NormalToSpectrum(hit.shadingCoordinateSystem.Binormal());
  case pe::DebugVisualizationMode::ShadingTangent:
    return NormalToSpectrum(hit.shadingCoordinateSystem.Tangent());
  case pe::DebugVisualizationMode::Sample:
    return Spectrum_t{sample.Get2D(0)[0], sample.Get2D(0)[1], 0.f};
  default:
    return Spectrum_t{};
  }
}

void pe::peDebugIntegrator::AllocateSamples(Sample &sample,
                                            const peScene &scene) {
  if (_mode != DebugVisualizationMode::Sample)
    return;
  sample.Add2D(1);
}

void pe::peDebugIntegrator::SetVisualizationMode(DebugVisualizationMode mode) {
  _mode = mode;
}
