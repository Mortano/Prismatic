#include "Integration\peDirectLightIntegrator.h"
#include "Math/peSampling.h"
#include "Scene/peScene.h"

pe::Spectrum_t pe::peDirectLightIntegrator::Estimate(
    const peScene &scene, const Sample &sample, const SceneHit &hit,
    const glm::vec3 &wo, const BSDF &bsdf, BxDFType flags,
    std::default_random_engine rng) {
  // Uniformly sample all lights in the scene
  Spectrum_t ret;
  for (size_t lightIdx = 0; lightIdx < scene.GetLights().size(); ++lightIdx) {
    auto &light = scene.GetLights()[lightIdx];
    auto numSamples = light->numSamples;
    Spectrum_t lightSummedIntensity;
    uint32_t numSamplesTaken = 0;
    for (uint32_t sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx) {
      auto lightSample = peLightSampleRandomValues{
          sample, _lightSampleOffsets[lightIdx], sampleIdx};
      auto bsdfSample =
          _bsdfSampleOffsets[lightIdx].ToBSDFSample(sample, sampleIdx);
      lightSummedIntensity += EstimateDirectLight(
          *light, scene, hit.hitPosition, hit.shadingCoordinateSystem,
          hit.hitNormal, wo, lightSample, bsdfSample, bsdf, flags);
    }

    ret += (lightSummedIntensity / numSamples);
  }
  return ret;
}

void pe::peDirectLightIntegrator::AllocateSamples(Sample &sample,
                                                  const peScene &scene) {
  const auto LightCount = scene.GetLights().size();
  _lightSampleOffsets.resize(LightCount);
  _bsdfSampleOffsets.resize(LightCount);
  for (auto idx = 0; idx < LightCount; ++idx) {
    auto &light = scene.GetLights()[idx];
    auto numSamples = light->numSamples;
    if (!numSamples) {
      PrismaticEngine.GetLogging()->LogWarning(
          "Light with zero samples will be ignored!");
      continue;
    }

    _lightSampleOffsets[idx] = LightSampleOffset{numSamples, sample};
    _bsdfSampleOffsets[idx] = BSDFSampleOffset{numSamples, sample};
  }
}
