#include "Rendering\Utility\peBxDF.h"
#include "DataStructures/peVector.h"
#include "Math/MathUtil.h"
#include "Math/peCoordSys.h"
#include "Math/peSampling.h"
#include "Type/peUnits.h"

pe::peBxDF::peBxDF(BxDFType type) : _type(type) {}

bool pe::peBxDF::HasFlags(BxDFType flags) const {
  return (static_cast<int>(_type) & static_cast<int>(flags)) ==
         static_cast<int>(_type);
}

pe::Spectrum_t pe::peBxDF::Sample_f(const glm::vec3 &wo, glm::vec3 &wi,
                                    const float rnd1, const float rnd2,
                                    float &pdf) const {
  // Sample the unit hemisphere with a cosine-weighted distribution
  wi = CosineSampleHemisphere(rnd1, rnd2);
  if (wi.z < 0.f)
    wi.z *= -1.f;
  pdf = Pdf(wo, wi);
  return Eval(wo, wi);
}

pe::Spectrum_t pe::peBxDF::rho(const glm::vec3 &wo,
                               const gsl::span<glm::vec2> &samples) const {
  return Spectrum_t{};
}

pe::Spectrum_t pe::peBxDF::rho(const gsl::span<glm::vec2> &samples1,
                               const gsl::span<glm::vec2> &samples2) const {
  return Spectrum_t{};
}

float pe::peBxDF::Pdf(const glm::vec3 &wo, const glm::vec3 &wi) const {
  return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * InvPi<float> : 0.f;
}

pe::peFresnelConductor::peFresnelConductor(const Spectrum_t &eta,
                                           const Spectrum_t &k)
    : _eta(eta), _k(k) {}

pe::Spectrum_t pe::peFresnelConductor::Eval(float cosi) const {
  return FresnelConductor(std::fabsf(cosi), _eta, _k);
}

pe::peFresnelDielectric::peFresnelDielectric(float etaIncident,
                                             float etaTransmitted)
    : _etaIndicent(etaIncident), _etaTransmitted(etaTransmitted) {}

pe::Spectrum_t pe::peFresnelDielectric::Eval(float cosi) const {
  return EvalFresnelDielectric(cosi, _etaIndicent, _etaTransmitted);
}

pe::peSpecularReflection::peSpecularReflection(const Spectrum_t &spectrum,
                                               const peFresnel &fresnel)
    : peBxDF(BxDFType::Reflection | BxDFType::Specular), _color(spectrum),
      _fresnel(fresnel) {}

pe::Spectrum_t pe::peSpecularReflection::Eval(const glm::vec3 &wo,
                                              const glm::vec3 &wi) const {
  return Spectrum_t{
      0.f}; // Can't evaluate specular reflection directly using wo and wi
}

pe::Spectrum_t pe::peSpecularReflection::Sample_f(const glm::vec3 &wo,
                                                  glm::vec3 &wi,
                                                  const float rnd1,
                                                  const float rnd2,
                                                  float &pdf) const {
  // Compute perfect specular reflection direction
  wi = {-wo.x, -wo.y, wo.z};
  pdf = 1.f;
  return _fresnel.Eval(CosTheta(wo)) * _color / AbsCosTheta(wi);
}

pe::peLambert::peLambert(const Spectrum_t &color)
    : peBxDF(BxDFType::Reflection | BxDFType::Diffuse), _color(color) {}

pe::Spectrum_t pe::peLambert::Eval(const glm::vec3 &wo,
                                   const glm::vec3 &wi) const {
  return _color * InvPi<float>;
}

pe::Spectrum_t pe::peLambert::rho(const glm::vec3 &wo,
                                  const gsl::span<glm::vec2> &samples) const {
  return _color; // Lambertian reflectance is constant in the hemisphere
}

pe::Spectrum_t pe::peLambert::rho(const gsl::span<glm::vec2> &samples1,
                                  const gsl::span<glm::vec2> &samples2) const {
  return _color; // Lambertian reflectance is constant in the hemisphere
}

pe::BSDFSample::BSDFSample(const glm::vec2 &dir, float component)
    : dir(dir), component(component) {}

pe::BSDF::BSDF() : _numBxdfs(0) {}

pe::BSDF::BSDF(const BSDF &other) : _numBxdfs(other._numBxdfs) {
  std::copy(other._bxdfs.begin(), other._bxdfs.begin() + other._numBxdfs,
            _bxdfs.begin());
}

pe::BSDF::BSDF(BSDF &&other) noexcept : _numBxdfs(other._numBxdfs) {
  std::copy(other._bxdfs.begin(), other._bxdfs.begin() + other._numBxdfs,
            _bxdfs.begin());
  other._numBxdfs = 0;
}

pe::BSDF &pe::BSDF::operator=(const BSDF &other) {
  _numBxdfs = other._numBxdfs;
  std::copy(other._bxdfs.begin(), other._bxdfs.begin() + other._numBxdfs,
            _bxdfs.begin());
  return *this;
}

pe::BSDF &pe::BSDF::operator=(BSDF &&other) noexcept {
  _numBxdfs = other._numBxdfs;
  std::copy(other._bxdfs.begin(), other._bxdfs.begin() + other._numBxdfs,
            _bxdfs.begin());
  other._numBxdfs = 0;
  return *this;
}

void pe::BSDF::Add(peBxDF const *bxdf) {
  if (_numBxdfs == MaxBxDF)
    throw std::runtime_error{"Maximum number of BxDFs exceeded!"};
  _bxdfs[_numBxdfs++] = bxdf;
}

uint32_t pe::BSDF::NumBxDFs() const { return _numBxdfs; }

uint32_t pe::BSDF::NumBxDFsWithFlags(BxDFType flags) const {
  return static_cast<uint32_t>(
      std::count_if(_bxdfs.begin(), _bxdfs.begin() + _numBxdfs,
                    [flags](auto bxdf) { return bxdf->HasFlags(flags); }));
}

pe::Spectrum_t pe::BSDF::Eval(const glm::vec3 &outgoingWorld,
                              const glm::vec3 &incomingWorld,
                              const peCoordSys &shadingCoordSys,
                              const glm::vec3 &geometryNormal,
                              BxDFType flags) const {
  const auto wi = shadingCoordSys.FromWorld(incomingWorld);
  const auto wo = shadingCoordSys.FromWorld(outgoingWorld);

  if (glm::dot(incomingWorld, geometryNormal) *
          glm::dot(outgoingWorld, geometryNormal) >
      0) {
    // If incoming and outgoing vectors are on the same side, ignore
    // transmission
    flags = (flags & ~BxDFType::Transmission);
  } else {
    // Else ignore reflection
    flags = (flags & ~BxDFType::Reflection);
  }

  // Sum up the spectra for all relevant brdfs
  auto ret = Spectrum_t{0.f};
  for (uint32_t idx = 0; idx < _numBxdfs; ++idx) {
    auto &bxdf = *_bxdfs[idx];
    if (!bxdf.HasFlags(flags))
      continue;
    ret += bxdf.Eval(wo, wi);
  }
  return ret;
}

pe::Spectrum_t pe::BSDF::Sample_f(const glm::vec3 &wo, glm::vec3 &wi,
                                  const peCoordSys &shadingCoordSys,
                                  const glm::vec3 &geometryNormal,
                                  const BSDFSample &sample, float &pdf,
                                  BxDFType flags, BxDFType &sampledType) const {
  const auto matchingComponents = NumBxDFsWithFlags(flags);
  if (!matchingComponents) {
    pdf = 0.f;
    return Spectrum_t{0.f};
  }

  auto which = std::min(
      static_cast<uint32_t>(std::floor(sample.component * matchingComponents)),
      matchingComponents - 1);

  auto &bxdf = [&]() -> const peBxDF & {
    auto count = which;
    for (uint32_t idx = 0; idx < _numBxdfs; ++idx) {
      if (_bxdfs[idx]->HasFlags(flags) && count-- == 0) {
        return *_bxdfs[idx];
      }
    }
    throw std::runtime_error{"Out of bounds!"};
  }();

  auto woLocal = shadingCoordSys.FromWorld(wo);
  glm::vec3 wiLocal;
  pdf = 0.f;
  auto f = bxdf.Sample_f(woLocal, wiLocal, sample.dir[0], sample.dir[1], pdf);
  if (pdf == 0.f)
    return Spectrum_t{0.f};
  sampledType = bxdf.Type();
  wi = shadingCoordSys.ToWorld(wiLocal);

  // Compute the average of all PDFs of all BxDFs that could have been used for
  // sampling  Only do this if the sampled BxDF is not a specular one
  if (!(bxdf.HasFlags(BxDFType::Specular)) && matchingComponents > 1) {
    pdf =
        std::accumulate(_bxdfs.begin(), _bxdfs.begin() + _numBxdfs, pdf,
                        [&](auto accum, peBxDF const *otherBxdf) {
                          if (otherBxdf != &bxdf && !otherBxdf->HasFlags(flags))
                            return accum;
                          return accum + otherBxdf->Pdf(woLocal, wiLocal);
                        });
  }
  if (matchingComponents > 1)
    pdf /= matchingComponents;

  if (!(bxdf.HasFlags(BxDFType::Specular))) {
    // This is just a call to BXDF::Eval again, but a bit more efficient because
    // we already  have the local incoming and outgoing vectors
    f = Spectrum_t{0.f};
    if (glm::dot(wi, geometryNormal) * glm::dot(wo, geometryNormal) > 0) {
      flags = (flags & ~BxDFType::Transmission);
    } else {
      flags = (flags & ~BxDFType::Reflection);
    }

    for (uint32_t idx = 0; idx < _numBxdfs; ++idx) {
      if (!_bxdfs[idx]->HasFlags(flags))
        continue;
      f += _bxdfs[idx]->Eval(woLocal, wiLocal);
    }
  }

  return f;
}

pe::Spectrum_t pe::BSDF::rho(std::default_random_engine &rnd, BxDFType flags,
                             uint32_t sqrtSamples) const {
  const auto numSamples = sqrtSamples * sqrtSamples;
  peVector<glm::vec2> samples1;
  samples1.resize(numSamples);
  peVector<glm::vec2> samples2;
  samples2.resize(numSamples);
  StratifiedSample2D(samples1, sqrtSamples, sqrtSamples, rnd);
  StratifiedSample2D(samples2, sqrtSamples, sqrtSamples, rnd);

  auto ret = Spectrum_t{0.f};
  for (uint32_t idx = 0; idx < _numBxdfs; ++idx) {
    auto &bxdf = *_bxdfs[idx];
    if (!bxdf.HasFlags(flags))
      continue;
    ret += bxdf.rho(samples1, samples2);
  }

  return ret;
}

pe::Spectrum_t pe::BSDF::rho(const glm::vec3 &wo,
                             std::default_random_engine &rnd, BxDFType flags,
                             uint32_t sqrtSamples) const {
  const auto numSamples = sqrtSamples * sqrtSamples;
  peVector<glm::vec2> samples;
  samples.resize(numSamples);
  StratifiedSample2D(samples, sqrtSamples, sqrtSamples, rnd);

  auto ret = Spectrum_t{0.f};
  for (uint32_t idx = 0; idx < _numBxdfs; ++idx) {
    auto &bxdf = *_bxdfs[idx];
    if (!bxdf.HasFlags(flags))
      continue;
    ret += bxdf.rho(wo, samples);
  }

  return ret;
}

float pe::BSDF::Pdf(const glm::vec3 &wo, const glm::vec3 &wi,
                    const peCoordSys &shadingCoordSys, BxDFType flags) const {
  auto ret = 0.f;
  uint32_t count = 0;

  const auto woLocal = shadingCoordSys.FromWorld(wo);
  const auto wiLocal = shadingCoordSys.FromWorld(wi);

  for (uint32_t idx = 0; idx < _numBxdfs; ++idx) {
    auto &bxdf = *_bxdfs[idx];
    if (!bxdf.HasFlags(flags))
      continue;
    ++count;
    ret += bxdf.Pdf(woLocal, wiLocal);
  }
  if (!count)
    return 0.f;
  return ret / count;
}

pe::RGB_32BitFloat pe::FresnelDielectric(float cosIncident,
                                         float cosTransmitted,
                                         const RGB_32BitFloat &etaIncident,
                                         const RGB_32BitFloat &etaTransmitted) {
  const auto rParallel =
      ((etaTransmitted * cosIncident) - (etaIncident * cosTransmitted)) /
      ((etaTransmitted * cosIncident) + (etaIncident * cosTransmitted));
  const auto rPerpendicular =
      ((etaIncident * cosIncident) - (etaTransmitted * cosTransmitted)) /
      ((etaIncident * cosIncident) + (etaTransmitted * cosTransmitted));
  return (rParallel * rParallel + rPerpendicular * rPerpendicular) / 2.f;
}

pe::RGB_32BitFloat pe::FresnelConductor(float cosIndicent,
                                        const RGB_32BitFloat &eta,
                                        const RGB_32BitFloat &k) {
  const auto tmp = (eta * eta + k * k) * cosIndicent * cosIndicent;
  const auto rParallel2 = (tmp - (2.f * eta * cosIndicent) + 1) /
                          (tmp + (2.f * eta * cosIndicent) + 1);
  const auto tmpF = eta * eta + k * k;
  const auto rPerpendicular2 =
      (tmpF - (2.f * eta * cosIndicent) + cosIndicent * cosIndicent) /
      (tmpF + (2.f * eta * cosIndicent) + cosIndicent * cosIndicent);
  return (rParallel2 + rPerpendicular2) / 2.f;
}

pe::RGB_32BitFloat pe::EvalFresnelDielectric(float cosIncident,
                                             float etaIncident,
                                             float etaTransmitted) {
  auto cosi = Clamp(cosIncident, -1.f, 1.f);
  auto isEntering = cosi > 0.f;
  auto ei = etaIncident, et = etaTransmitted;
  if (!isEntering)
    std::swap(ei, et);
  // Snells law
  const auto sint = (ei / et) * std::sqrtf(std::max(0.f, 1.f - cosi * cosi));
  if (sint >= 1.f) {
    // Total internal reflection
    return RGB_32BitFloat{1.f};
  } else {
    auto cost = std::sqrtf(std::max(0.f, 1.f - sint * sint));
    return FresnelDielectric(std::fabs(cosi), cost, RGB_32BitFloat{etaIncident},
                             RGB_32BitFloat{etaTransmitted});
  }
}
