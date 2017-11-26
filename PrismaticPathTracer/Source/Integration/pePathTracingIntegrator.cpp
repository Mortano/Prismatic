#include "Integration\pePathTracingIntegrator.h"
#include "Math/peSampling.h"
#include "Scene/peScene.h"

static pe::Spectrum_t
UniformSampleOneLight(const pe::peScene &scene, const glm::vec3 &point,
                      const pe::peCoordSys &shadingCoordinateSystem,
                      const glm::vec3 &normal, const glm::vec3 &wo,
                      const pe::peLightSampleRandomValues &lightSample,
                      const pe::BSDFSample &bsdfSample, const pe::BSDF &bsdf,
                      pe::BxDFType flags) {
  auto numLights = static_cast<uint32_t>(scene.GetLights().size());
  if (!numLights)
    return pe::Spectrum_t{0, 0, 0};
  auto lightIdx =
      static_cast<uint32_t>(floor(bsdfSample.component * numLights));
  lightIdx = std::min(lightIdx, numLights - 1);
  auto light = scene.GetLights()[lightIdx].get();
  return pe::EstimateDirectLight(*light, scene, point, shadingCoordinateSystem,
                                 normal, wo, lightSample, bsdfSample, bsdf,
                                 flags);
}

pe::pePathTracingIntegrator::pePathTracingIntegrator(uint32_t maxDepth)
    : _maxDepth(maxDepth), _numSamplesPerPixel(1) {}

static pe::Spectrum_t NormalToSpectrum(const glm::vec3 &normal) {
  return pe::Spectrum_t{0.5f + normal.x / 2, 0.5f + normal.y / 2,
                        0.5f + normal.z / 2};
}

pe::Spectrum_t pe::pePathTracingIntegrator::Estimate(
    const peScene &scene, const Sample &sample, const SceneHit &hit,
    const glm::vec3 &wo, const BSDF &bsdf, BxDFType flags,
    std::default_random_engine rng) {

  // glm::vec3 wi;
  // float pdf;
  // BxDFType sampledFlags;
  // auto bsdfSample = _pathSampleOffsets[0].ToBSDFSample(sample, 0);
  // auto f = bsdf.Sample_f(wo, wi, hit.shadingCoordinateSystem, hit.hitNormal,
  //                       bsdfSample, pdf, flags, sampledFlags);
  //
  // return NormalToSpectrum(wi);

  auto throughput = Spectrum_t{1, 1, 1};
  auto totalLight = Spectrum_t{0, 0, 0};
  std::uniform_real_distribution<float> dist;
  const BSDF *currentBsdf = &bsdf;
  Ray ray;
  SceneHit currentHit = hit;
  for (uint32_t bounces = 0;; ++bounces) {
    // TODO Emissive light
    // TODO Specular
    // TODO Multiple samples
    if (bounces < SampleDepth) {
      auto lightSample =
          peLightSampleRandomValues{sample, _lightSampleOffsets[bounces], 0};
      auto bsdfSample = _bsdfSampleOffsets[bounces].ToBSDFSample(sample, 0);
      totalLight += throughput *
                    UniformSampleOneLight(scene, currentHit.hitPosition,
                                          currentHit.shadingCoordinateSystem,
                                          currentHit.hitNormal, wo, lightSample,
                                          bsdfSample, *currentBsdf, flags);
    } else {
      peLightSampleRandomValues lightSample{{dist(rng), dist(rng)}};
      BSDFSample bsdfSample{{dist(rng), dist(rng)}, dist(rng)};
      totalLight += throughput *
                    UniformSampleOneLight(scene, currentHit.hitPosition,
                                          currentHit.shadingCoordinateSystem,
                                          currentHit.hitNormal, wo, lightSample,
                                          bsdfSample, *currentBsdf, flags);
    }

    BSDFSample outgoingBSDFSample;
    if (bounces < SampleDepth) {
      outgoingBSDFSample = _pathSampleOffsets[bounces].ToBSDFSample(sample, 0);
    } else {
      outgoingBSDFSample.dir = {dist(rng), dist(rng)};
      outgoingBSDFSample.component = dist(rng);
    }

    glm::vec3 wi;
    float pdf;
    BxDFType sampledFlags;
    auto f = currentBsdf->Sample_f(wo, wi, currentHit.shadingCoordinateSystem,
                                   currentHit.hitNormal, outgoingBSDFSample,
                                   pdf, flags, sampledFlags);
    // If there is no contribution due to the BSDF, we are done
    if (f.IsBlack() || pdf == 0.f)
      break;

    throughput *= (f * std::abs(glm::dot(wi, currentHit.hitNormal)) / pdf);
    ray.direction = wi;
    ray.origin = currentHit.hitPosition + (0.001f * wi);
    ray.t = FLT_MAX;

    // Check if we must terminate the path. After 'SampleDepth', we continue the
    // path with a given probability
    if (bounces > SampleDepth) {
      auto continueProbability = std::min(0.5f, throughput.g());
      if (dist(rng) > continueProbability)
        break;
      throughput /= continueProbability;
    }
    if (bounces == _maxDepth)
      break;

    // Find the next intersection point
    auto isect = scene.GetIntersection(ray);
    if (!isect)
      break;

    currentHit = *isect;
  }

  return totalLight;
}

void pe::pePathTracingIntegrator::AllocateSamples(Sample &sample,
                                                  const peScene &scene) {

  // TODO Offsets for light index
  _lightSampleOffsets.resize(SampleDepth * _numSamplesPerPixel);
  _bsdfSampleOffsets.resize(SampleDepth * _numSamplesPerPixel);
  _pathSampleOffsets.resize(SampleDepth * _numSamplesPerPixel);

  for (uint32_t idx = 0; idx < SampleDepth; ++idx) {
    _lightSampleOffsets[idx] = LightSampleOffset{1, sample};
    _bsdfSampleOffsets[idx] = BSDFSampleOffset{1, sample};
    _pathSampleOffsets[idx] = BSDFSampleOffset{1, sample};
  }
}
