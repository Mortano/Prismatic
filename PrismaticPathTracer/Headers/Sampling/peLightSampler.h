#pragma once
#include "Components/peLightComponent.h"
#include "Rendering/Utility/peBxDF.h"
#include "Util/Ray.h"

namespace pe {
class peScene;

//! \brief Helper structure for tracing shadow rays. By using this structure,
//! integrators can defer the computation of shadow rays by checking if
//! illumination from a given direction is relevant first
struct VisibilityTester {

  void SetSegment(const glm::vec3 &start, float epsilonStart,
                  const glm::vec3 &end, float epsilonEnd);

  void SetRay(const glm::vec3 &origin, float epsilon, const glm::vec3 &dir);

  bool Unoccluded(const peScene &scene) const;

  Ray ray;
};

struct LightSampleOffset;

//! \brief Encapsulates random values for sampling lights
struct peLightSampleRandomValues {
  peLightSampleRandomValues() = default;
  peLightSampleRandomValues(const Sample &sample,
                            const LightSampleOffset &offset,
                            uint32_t sampleIndex);
  explicit peLightSampleRandomValues(const glm::vec2 &rnd);

  glm::vec2 rnd;
};

struct LightSampleOffset {
  LightSampleOffset() = default;
  LightSampleOffset(uint32_t numSamples, Sample &sample);

  uint32_t numSamples, offset;
};

//! \brief Result of the peLightSampler::SampleLightRay method
struct SampleLightRayResult {
  Spectrum_t intensity;
  Ray ray;
  glm::vec3 normal;
  float pdf;
};

//! \brief Interface for sampling lights
struct peLightSampler {
  peLightSampler(uint32_t numSamples, bool isDeltaLight);
  virtual ~peLightSampler() {}

  //! \brief Samples the incoming light from this light at the given point
  //! \param point Point at which incident light from this light shall be
  //! sampled \param epsilon Epsilon value for shadow ray \param rnd Random
  //! values for sampling \param wi Incoming vector, will be calculated by this
  //! function \param pdf Probability density function \param visibilityTester
  //! Visibility helper structure to defer calculating the shadow ray \returns
  //! Incident light at the given point from this light (assuming that the light
  //! were visible)
  virtual Spectrum_t
  SampleLightAtPoint(const glm::vec3 &point, float epsilon,
                     const peLightSampleRandomValues &rnd, glm::vec3 &wi,
                     float &pdf, VisibilityTester &visibilityTester) const = 0;

  //! \brief Returns the probability density with respect to solid angle for
  //! sampling the direction wi from point \param point Point to sample from
  //! \param wi Direction of sampling
  //! \returns PDF
  virtual float Pdf(const glm::vec3 &point, const glm::vec3 &wi) const = 0;

  //! \brief Samples a light ray coming from this light
  virtual SampleLightRayResult
  SampleLightRay(const peScene &scene, const peLightSampleRandomValues &rnd,
                 float rnd1, float rnd2) const = 0;

  //! \brief Returns the total power of this light
  //! \param scene Scene
  //! \returns Total power of this light
  virtual Spectrum_t Power(const peScene &scene) const = 0;

  //! \brief Is this light a delta light, i.e. an infinitly small light soure?
  bool IsDeltaLight() const { return _isDeltaLight; }

  const uint32_t numSamples;

private:
  const bool _isDeltaLight;
};

//! \brief Sampler for point lights
class pePointLightSampler : public peLightSampler {
public:
  explicit pePointLightSampler(const pePointLightComponent &light);

  Spectrum_t
  SampleLightAtPoint(const glm::vec3 &point, float epsilon,
                     const peLightSampleRandomValues &rnd, glm::vec3 &wi,
                     float &pdf,
                     VisibilityTester &visibilityTester) const override;

  Spectrum_t Power(const peScene &scene) const override;

  float Pdf(const glm::vec3 &point, const glm::vec3 &wi) const override;
  SampleLightRayResult SampleLightRay(const peScene &scene,
                                      const peLightSampleRandomValues &rnd,
                                      float rnd1, float rnd2) const override;

private:
  const pePointLightComponent &_light;
};

//! \brief Estimates direct lighting contribution from the given light
pe::Spectrum_t EstimateDirectLight(
    const pe::peLightSampler &light, const pe::peScene &scene,
    const glm::vec3 &point, const pe::peCoordSys &shadingCoordinateSystem,
    const glm::vec3 &normal, const glm::vec3 &wo,
    const pe::peLightSampleRandomValues &lightSample,
    const pe::BSDFSample &bsdfSample, const pe::BSDF &bsdf, pe::BxDFType flags);

} // namespace pe
