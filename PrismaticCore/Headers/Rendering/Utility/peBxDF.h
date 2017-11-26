#pragma once
#include "Type/peColor.h"
#include "Type\peBitmask.h"
#include "peCoreDefs.h"

#include <glm/detail/type_vec2.hpp>
#undef max
#include <gsl.h>
#include <random>

#pragma warning(push)
#pragma warning(disable : 4251)

/*
 * Everything in here is taking more or less one-to-one from 'Physically based
 * rendering' (Pharr, Humphreys, 2010)
 */

namespace pe {
struct Sample;
class peCoordSys;

//! \brief Represents a color spectrum. We will use RGB with floating-point
//! precision here
using Spectrum_t = RGB_32BitFloat;

//! \brief Types of BRDFs and BTDFs
enum class BxDFType {
  Reflection = 1 << 0,
  Transmission = 1 << 1,
  Diffuse = 1 << 2,
  Glossy = 1 << 3,
  Specular = 1 << 4,
  AllTypes = Diffuse | Glossy | Specular,
  AllReflection = Reflection | AllTypes,
  AllTransmission = Transmission | AllTypes,
  All = AllReflection | AllTransmission
};

template <> struct EnableEnumBitmask<BxDFType> : std::true_type {};

//! \brief BRDF or BTDF base class
class PE_CORE_API peBxDF {
public:
  virtual ~peBxDF() = default;
  explicit peBxDF(BxDFType type);

  bool HasFlags(BxDFType flags) const;

  auto Type() const { return _type; }

  //! \brief Evaluate this BxDF for the given outgoing and incoming vectors
  //! \param wo Outgoing vector
  //! \param wi Incoming vector
  //! \returns Value of the distribution function for the two vectors
  virtual Spectrum_t Eval(const glm::vec3 &wo, const glm::vec3 &wi) const = 0;

  //! \brief Sampling function for BxDF distributions that utilize delta
  //! distributions. Here, the incident direction wi cannot
  //!        be known to the user, hence the BxDF computes it
  //! \param wo Outgoing vector
  //! \param wi Incoming vector, will be computed by the BxDF
  //! \param rnd1 First uniform random variable
  //! \param rnd2 Second uniform random variable
  //! \param pdf Probability density function for this BxDF and the given pair
  //! of vectors \returns Value of the distribution function for the two vectors
  virtual Spectrum_t Sample_f(const glm::vec3 &wo, glm::vec3 &wi,
                              const float rnd1, const float rnd2,
                              float &pdf) const;

  //! \brief Computes the hemishperical-directional reflectance, which is the
  //! total reflection in the given direction due to constant illumination over
  //! the hemisphere. Not every BxDF will be able to compute this in closed
  //! form, so they will use something like a monte carlo algorithm to compute
  //! it, hence the <paramref name="samples"/> member
  //! \param wo Outgoing vector
  //! \param samples Samples
  //! \returns Hemispherical reflection around <paramref name="wo"/>
  virtual Spectrum_t rho(const glm::vec3 &wo,
                         const gsl::span<glm::vec2> &samples) const;

  //! \brief Computes the hemispherical-hemispherical reflectance, which is the
  //! fraction of incident light reflected by the surface when the incident
  //! light is the same from all directions. Not every BxDF will be able to
  //! compute this in closed form, so they will use something like a monte carlo
  //! algorithm to compute it
  //! \param samples1 First set of samples for monte carlo method
  //! \param samples2 Second set of samples for monte carlo method
  //! \returns Direction-independent reflectance
  virtual Spectrum_t rho(const gsl::span<glm::vec2> &samples1,
                         const gsl::span<glm::vec2> &samples2) const;

  //! \brief Returns the probability density function for the given pairs of
  //! vectors \param wo Outgoing vector in shading space \param wi Incoming
  //! vector in shading space \returns PDF
  virtual float Pdf(const glm::vec3 &wo, const glm::vec3 &wi) const;

private:
  const BxDFType _type;
};

#pragma region Fresnel

//! \brief Helper class to encapsulate fresnel reflectance
struct PE_CORE_API peFresnel {
  virtual ~peFresnel() {}
  virtual Spectrum_t Eval(float cosi) const = 0;
};

//! \brief Fresnel reflectance for a conductor
class PE_CORE_API peFresnelConductor : public peFresnel {
public:
  //! \brief Initializes this FresnelConductor structure with the given index of
  //! refraction and absorption
  //! \param eta Index of refraction of conductor
  //! \param k Absorption index of conductor
  peFresnelConductor(const Spectrum_t &eta, const Spectrum_t &k);
  Spectrum_t Eval(float cosi) const override;

private:
  const Spectrum_t _eta;
  const Spectrum_t _k;
};

//! \brief Fresnel reflectance for dielectric material
class PE_CORE_API peFresnelDielectric : public peFresnel {
public:
  //! \brief Initializes this FresnelDielectric structure with the given
  //! incident and transmitted medium indices of refraction
  //! \param etaIncident Index of refraction for incident material
  //! \param etaTransmitted Index of refraction for transmitted material
  peFresnelDielectric(float etaIncident, float etaTransmitted);
  Spectrum_t Eval(float cosi) const override;

private:
  const float _etaIndicent, _etaTransmitted;
};

#pragma endregion

//! \brief BRDF for specular reflection
class PE_CORE_API peSpecularReflection : public peBxDF {
public:
  peSpecularReflection(const Spectrum_t &spectrum, const peFresnel &fresnel);

  Spectrum_t Eval(const glm::vec3 &wo, const glm::vec3 &wi) const override;
  Spectrum_t Sample_f(const glm::vec3 &wo, glm::vec3 &wi, const float rnd1,
                      const float rnd2, float &pdf) const override;

private:
  const Spectrum_t _color;
  const peFresnel &_fresnel;
};

//! \brief Lambertian diffuse reflection
class PE_CORE_API peLambert : public peBxDF {
public:
  explicit peLambert(const Spectrum_t &color);
  Spectrum_t Eval(const glm::vec3 &wo, const glm::vec3 &wi) const override;

  Spectrum_t rho(const glm::vec3 &wo,
                 const gsl::span<glm::vec2> &samples) const override;
  Spectrum_t rho(const gsl::span<glm::vec2> &samples1,
                 const gsl::span<glm::vec2> &samples2) const override;

private:
  const Spectrum_t _color;
};

//! \brief Random values for BSDF sampling
struct PE_CORE_API BSDFSample {
  BSDFSample() = default;
  BSDFSample(const glm::vec2 &dir, float component);
  template <typename Rnd> explicit BSDFSample(Rnd &rng) {
    std::uniform_real_distribution<float> dist{0.f, 1.f};
    dir = {dist(rng), dist(rng)};
    component = dist(rng);
  }

  glm::vec2 dir;
  float component;
};

//! \brief Bidirectional scattering distribution function. Determines surface
//! properties of an object
class PE_CORE_API BSDF {
public:
  BSDF();

  BSDF(const BSDF &other);
  BSDF(BSDF &&other) noexcept;

  BSDF &operator=(const BSDF &);
  BSDF &operator=(BSDF &&) noexcept;

  void Add(peBxDF const *bxdf);

  uint32_t NumBxDFs() const;
  uint32_t NumBxDFsWithFlags(BxDFType flags) const;

  //! \brief Evaluate the BSDF for the given set of incoming and outgoing
  //! vectors \param outgoingWorld Outgoing vector in world space \param
  //! incomingWorld Incoming vector in world space \param shadingCoordSys
  //! Shading coordinate system \param flags Types of BxDFs to sample \param
  //! geometricNormal The normal of the actual geometry for the evaluated point
  //! \returns Evaluated spectrum
  Spectrum_t Eval(const glm::vec3 &outgoingWorld,
                  const glm::vec3 &incomingWorld,
                  const peCoordSys &shadingCoordSys,
                  const glm::vec3 &geometricNormal, BxDFType flags) const;

  //! \brief Sample this BSDF by evaluating a random BxDF
  Spectrum_t Sample_f(const glm::vec3 &wo, glm::vec3 &wi,
                      const peCoordSys &shadingCoordSys,
                      const glm::vec3 &geometryNormal, const BSDFSample &sample,
                      float &pdf, BxDFType flags, BxDFType &sampledType) const;

  //! \brief Sums up the hemispherical-hemispherical reflectance values of
  //! all assigned BxDFs \param rnd Random number generator for monte carlo
  //! method \param flags Types of BxDFs to sample \param sqrtSamples
  //! Sampling parameter for monte carlo \returns Summed reflectance
  Spectrum_t rho(std::default_random_engine &rnd,
                 BxDFType flags = BxDFType::All,
                 uint32_t sqrtSamples = 6) const;

  //! \brief Sums up the hemispherical-directional reflectance values of all
  //! assigned BxDFs \param wo Outgoing direction \param rnd Random number
  //! generator for monte carlo method \param flags Types of BxDFs to sample
  //! \param sqrtSamples Sampling parameter for monte carlo
  //! \returns Summed reflectance
  Spectrum_t rho(const glm::vec3 &wo, std::default_random_engine &rnd,
                 BxDFType flags = BxDFType::All,
                 uint32_t sqrtSamples = 6) const;

  //! \brief Probability density function of this BSDF for the given pair of
  //! vectors \param wo Outgoing vector in world space \param wi Incoming vector
  //! in world space
  float Pdf(const glm::vec3 &wo, const glm::vec3 &wi,
            const peCoordSys &shadingCoordSys,
            BxDFType flags = BxDFType::All) const;

private:
  constexpr static size_t MaxBxDF = 8;
  uint32_t _numBxdfs;
  std::array<peBxDF const *, MaxBxDF> _bxdfs;
};

#pragma region HelperFunctions

//! \brief Computes fresnel reflectance for dielectric materials
//! \param cosIncident Cosine of angle of incident direction
//! \param cosTransmitted Cosine of angle of transmitted direction
//! \param etaIncident Index of refraction for incident medium
//! \param etaTransmitted Index of refraction for transmitted medium
//! \returns Fresnel reflectance
Spectrum_t PE_CORE_API FresnelDielectric(float cosIncident,
                                         float cosTransmitted,
                                         const Spectrum_t &etaIncident,
                                         const Spectrum_t &etaTransmitted);

//! \brief Computes the fresnel reflectance for a conducting material
//! \param cosIndicent Cosine of the angle of incident direction
//! \param eta Index of refraction of the conductor
//! \param k Absorption coefficient
//! \returns Fresnel reflectance
Spectrum_t PE_CORE_API FresnelConductor(float cosIndicent,
                                        const Spectrum_t &eta,
                                        const Spectrum_t &k);

//! \brief Helper to evaluate fresnel shading for dieletric materials
//! \param cosIncident Cosine of angle of incident direction
//! \param etaIncident Index of refraction of incident medium
//! \param etaTransmitted Index of refraction of transmitted medium
//! \returns Fresnel reflectance
Spectrum_t PE_CORE_API EvalFresnelDielectric(float cosIncident,
                                             float etaIncident,
                                             float etaTransmitted);

#pragma endregion

} // namespace pe

#pragma warning(pop)
