#include "Sampling\peLightSampler.h"
#include "Math/peSampling.h"
#include "Sampling/peSampler.h"
#include "Scene/peScene.h"
#include "Type/peUnits.h"

void pe::VisibilityTester::SetSegment(const glm::vec3 &start,
                                      const float epsilonStart,
                                      const glm::vec3 &end,
                                      const float epsilonEnd) {
  const auto dist = glm::length(end - start);
  ray.direction = glm::normalize(end - start);
  ray.origin = start + (ray.direction * epsilonStart);
  ray.t = dist * (1.f - epsilonEnd);
}

void pe::VisibilityTester::SetRay(const glm::vec3 &origin, const float epsilon,
                                  const glm::vec3 &dir) {
  ray.origin = origin + (dir * epsilon);
  ray.direction = dir;
  ray.t = std::numeric_limits<float>::max();
}

bool pe::VisibilityTester::Unoccluded(const peScene &scene) const {
  return !scene.Intersects(ray);
}

pe::peLightSampleRandomValues::peLightSampleRandomValues(
    const Sample &sample, const LightSampleOffset &offset,
    uint32_t sampleIndex) {
  const auto sampleBuffer = sample.Get2D(offset.offset);
  rnd.x = sampleBuffer[2 * sampleIndex];
  rnd.y = sampleBuffer[2 * sampleIndex + 1];
}

pe::peLightSampleRandomValues::peLightSampleRandomValues(const glm::vec2 &rnd)
    : rnd(rnd) {}

pe::LightSampleOffset::LightSampleOffset(uint32_t numSamples, Sample &sample)
    : numSamples(numSamples) {
  offset = sample.Add2D(numSamples);
}

pe::peLightSampler::peLightSampler(uint32_t numSamples, bool isDeltaLight)
    : numSamples(numSamples), _isDeltaLight(isDeltaLight) {}

pe::pePointLightSampler::pePointLightSampler(const pePointLightComponent &light)
    : peLightSampler(light.numSamples, true), _light(light) {}

pe::Spectrum_t pe::pePointLightSampler::SampleLightAtPoint(
    const glm::vec3 &point, float epsilon, const peLightSampleRandomValues &rnd,
    glm::vec3 &wi, float &pdf, VisibilityTester &visibilityTester) const {
  const auto pointToLight = _light.position - point;
  const auto dist = glm::length(pointToLight);
  wi = glm::normalize(pointToLight);
  pdf = 1.f;
  visibilityTester.SetSegment(point, epsilon, _light.position, 0.f);
  return _light.intensity / (dist * dist);
}

pe::Spectrum_t pe::pePointLightSampler::Power(const peScene &scene) const {
  return 4.f * Pi<float> *
         _light.intensity; // Light integrated over the whole sphere
}

float pe::pePointLightSampler::Pdf(const glm::vec3 &point,
                                   const glm::vec3 &wi) const {
  return 0.f; // Point light uses a delta distribution as it is just a single
              // point
}

pe::SampleLightRayResult
pe::pePointLightSampler::SampleLightRay(const peScene &scene,
                                        const peLightSampleRandomValues &rnd,
                                        float rnd1, float rnd2) const {
  const auto rayDir = UniformSampleSphere(rnd1, rnd2);
  return {_light.intensity,
          {_light.position, rayDir, std::numeric_limits<float>::max()},
          rayDir,
          UniformSampleSpherePdf()};
}

pe::Spectrum_t
pe::EstimateDirectLight(const pe::peLightSampler &light,
                        const pe::peScene &scene, const glm::vec3 &point,
                        const pe::peCoordSys &shadingCoordinateSystem,
                        const glm::vec3 &normal, const glm::vec3 &wo,
                        const pe::peLightSampleRandomValues &lightSample,
                        const pe::BSDFSample &bsdfSample, const pe::BSDF &bsdf,
                        pe::BxDFType flags) {
  auto ret = pe::Spectrum_t{0.f};
  glm::vec3 wi;
  float lightPdf, bsdfPdf;
  pe::VisibilityTester visibility;
  auto lightIntensity = light.SampleLightAtPoint(point, 0.001f, lightSample, wi,
                                                 lightPdf, visibility);

  if (lightPdf <= 0.f || lightIntensity.IsBlack())
    return ret;
  auto bsdfIntensity =
      bsdf.Eval(wo, wi, shadingCoordinateSystem, normal, flags);
  if (bsdfIntensity.IsBlack())
    return ret;

  // Now we evaluate the occlusion
  if (!visibility.Unoccluded(scene))
    return ret;

  // Sample light source with multiple importance sampling
  if (light.IsDeltaLight()) {
    ret += bsdfIntensity * lightIntensity *
           (std::fabsf(glm::dot(wi, normal)) / lightPdf);
  } else {
    bsdfPdf = bsdf.Pdf(wo, wi, shadingCoordinateSystem, flags);
    // Weight for multiple importance sampling
    auto weight = pe::PowerHeuristic(1, lightPdf, 1, bsdfPdf);
    ret += bsdfIntensity * lightIntensity *
           (std::fabsf(glm::dot(wi, normal)) * weight / lightPdf);
  }

  if (light.IsDeltaLight())
    return ret;

  // Sample BSDF with multiple importance sampling
  pe::BxDFType sampledType;
  auto bsdfF = bsdf.Sample_f(wo, wi, shadingCoordinateSystem, normal,
                             bsdfSample, bsdfPdf, flags, sampledType);

  if (bsdfF.IsBlack() || bsdfPdf == 0.f)
    return ret;

  auto weight = 1.f;
  if (!(sampledType & pe::BxDFType::Specular)) {
    lightPdf = light.Pdf(point, wi);
    if (lightPdf == 0.f)
      return ret;
    weight = pe::PowerHeuristic(1, bsdfPdf, 1, lightPdf);
  }

  // TODO Check that ray in direction wi actually hits the light source for
  // non-delta light

  return ret;
}
