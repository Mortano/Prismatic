#pragma once

#include "Type\peUnits.h"
#include "peUtilDefs.h"

#include <glm\common.hpp>
#include <random>
#include <span.h>

namespace pe {

//! \brief Computes stratified samples in 1D
//! \param samples Range where the samples will be stored in
//! \param rng Random number generator
//! \param jitter Enable jittering? If not enabled, the samples will all lie on
//! a fixed grid
template <typename Rng>
void StratifiedSample1D(gsl::span<float> samples, Rng &rng,
                        const bool jitter = true) {
  const auto numSamples = gsl::narrow_cast<uint32_t>(samples.size());
  const auto dx = 1.f / numSamples;
  std::uniform_real_distribution<float> dist;
  for (uint32_t idx = 0; idx < numSamples; ++idx) {
    auto delta = jitter ? dist(rng) : 0.5f;
    samples[idx] = (idx + delta) * dx;
  }
}

//! \brief Computes stratified samples in 2D. Computes two samples per point,
//! one stratified in X and one stratified in Y direction \param samples Range
//! where the samples will be stored in \param samplesX Sample count in
//! X-direction \param samplesY Sample count in Y-direction \param rng Random
//! number generator \param jitter Enable jittering? If not enabled, the samples
//! will all lie on a fixed grid
template <typename Rng>
void StratifiedSample2D(gsl::span<glm::vec2> samples, const uint32_t samplesX,
                        const uint32_t samplesY, Rng &rng,
                        const bool jitter = true) {
  if (samples.size() < (samplesX * samplesY))
    throw std::runtime_error{
        "Need at least samplesX * samplesY samples in range!"};
  const auto dx = 1.f / samplesX;
  const auto dy = 1.f / samplesY;
  std::uniform_real_distribution<float> dist{0.f, 1.f};
  uint32_t idx = 0;
  for (uint32_t y = 0; y < samplesY; ++y) {
    for (uint32_t x = 0; x < samplesX; ++x) {
      auto jx = jitter ? dist(rng) : 0.5f;
      auto jy = jitter ? dist(rng) : 0.5f;
      samples[idx++] = {(x + jx) * dx, (y + jy) * dy};
    }
  }
}

//! \brief Latin hypercube algorithm for generating samples in N dimensions
//! \param samplesBegin Begin iterator to samples range
//! \param samplesEnd End iterator to samples range
//! \param dimensions Number of dimensions
//! \param rng Random number generator
template <typename Iter, typename Rng>
void LatinHypercube(Iter samplesBegin, Iter samplesEnd,
                    const uint32_t dimensions, Rng &rng) {
  const auto numSamples = static_cast<uint32_t>(
      std::distance(samplesBegin, samplesEnd) / dimensions);
  if (!numSamples)
    return;

  std::uniform_real_distribution<float> floatDist{0.f, 1.f};
  std::uniform_int_distribution<uint32_t> intDist;

  const auto delta = 1.f / numSamples;
  // Generate random samples on diagonal
  for (uint32_t idx = 0; idx < numSamples; ++idx) {
    for (uint32_t dim = 0; dim < dimensions; ++dim) {
      samplesBegin[dimensions * idx + dim] =
          (static_cast<float>(idx) + floatDist(rng)) * delta;
    }
  }

  // Permute samples
  for (uint32_t dim = 0; dim < dimensions; ++dim) {
    for (uint32_t idx = 0; idx < numSamples; ++idx) {
      auto other = idx + (intDist(rng) % (numSamples - idx));
      std::swap(samplesBegin[dimensions * idx + dim],
                samplesBegin[dimensions * other + dim]);
    }
  }
}

//! \brief Generates a new random vector by randomly sampling the hemisphere
//! around the positive Z-axis
//! \param rnd1 First uniform random variable
//! \param rnd2 Second uniform random variable
//! \returns Random vector in hemisphere
glm::vec3 PE_UTIL_API UniformSampleHemisphere(const float rnd1,
                                              const float rnd2);

//! \brief Generates a new random vector by randomly sampling the hemisphere
//! around the positive Z-axis
//! \param rng Random number generator
//! \returns Random vector in hemisphere
template <typename Rng> glm::vec3 UniformSampleHemisphere(Rng &rng) {
  std::uniform_real_distribution<float> dist{0.f, 1.f};
  const auto rnd1 = dist(rng);
  const auto rnd2 = dist(rng);
  return UniformSampleHemisphere(rnd1, rnd2);
}

//! \returns The probability density function for sampling a uniform
//! vector in a hemisphere
float PE_UTIL_API UniformSampleHemispherePdf();

//! \brief Generates a new random vector by randomly sampling the unit sphere
//! \param rnd1 First uniform random variable
//! \param rnd2 Second uniform random variable
//! \returns Random vector in sphere
glm::vec3 PE_UTIL_API UniformSampleSphere(const float rnd1, const float rnd2);

//! \brief Generates a new random vector by randomly sampling the unit sphere
//! \param rng Random number generator
//! \returns Random vector in sphere
template <typename Rng> glm::vec3 UniformSampleSphere(Rng &rng) {
  std::uniform_real_distribution<float> dist{0.f, 1.f};
  const auto rnd1 = dist(rng);
  const auto rnd2 = dist(rng);
  return UniformSampleSphere(rnd1, rnd2);
}

//! \returns The probability density function for sampling a vector in the
//! unit sphere
float PE_UTIL_API UniformSampleSpherePdf();

//! \brief Generates a new random point on the unit disk using the concentric
//! mapping method by (Shirley, 1997)
//! \param rnd1 First uniform random variable
//! \param rnd2 Second uniform random variable
//! \returns Random point on unit disk
glm::vec2 PE_UTIL_API ConcentricSampleDisk(const float rnd1, const float rnd2);

//! \brief Generates a new random point on the unit disk using the concentric
//! mapping method by (Shirley, 1997)
//! \param rng Random number generator
//! \returns Random point on unit disk
template <typename Rng> glm::vec2 ConcentricSampleDisk(Rng &rng) {
  std::uniform_real_distribution<float> dist{-1.f, 1.f};
  const auto rndX = dist(rng);
  const auto rndY = dist(rng);
  return ConcentricSampleDisk(rndX, rndY);
}

//! \brief Generates a new random point on the hemisphere around the positive
//! Z-axis using a cosine-weighted distribution
//! \param rnd1 First uniform random variable
//! \param rnd2 Second uniform random variable
//! \returns New random point
glm::vec3 PE_UTIL_API CosineSampleHemisphere(const float rnd1,
                                             const float rnd2);

//! \brief Generates a new random point on the hemisphere around the positive
//! Z-axis using a cosine-weighted distribution
//! \param rng Random number generator
//! \returns New random point
template <typename Rng> glm::vec3 CosineSampleHemisphere(Rng &rng) {
  auto rndDisk = ConcentricSampleDisk(rng);
  auto z = std::sqrtf(
      std::max(0.f, 1.f - rndDisk.x * rndDisk.x - rndDisk.y * rndDisk.y));
  return {rndDisk.x, rndDisk.y, z};
}

//! \brief Returns the probability density function for cosine-weighted
//! hemisphere sampling \param cosTheta Cosine of the angle theta \param phi
//! Angle phi \returns PDF
float PE_UTIL_API CosineSampleHemispherePdf(float cosTheta, float phi);

//! \brief Power-heuristic for multipe importance sampling (p. 693 in
//! Physically-based rendering)
float PE_UTIL_API PowerHeuristic(int nf, float fPdf, int ng, float gPdf);

void PE_UTIL_API Sample02(uint32_t n, const uint32_t scramble[2],
                          float sample[2]);

float PE_UTIL_API VanDerCorput(uint32_t n, uint32_t scramble);

float PE_UTIL_API Sobol2(uint32_t n, uint32_t scramble);

template <typename T, typename Rng>
void Shuffle(T *samp, uint32_t count, uint32_t dims, Rng &rng) {
  std::uniform_int_distribution<uint32_t> dist;
  for (uint32_t i = 0; i < count; ++i) {
    uint32_t other = i + (dist(rng) % (count - i));
    for (uint32_t j = 0; j < dims; ++j)
      swap(samp[dims * i + j], samp[dims * other + j]);
  }
}

template <typename Rng>
void LDShuffleScrambled1D(int nSamples, int nPixel, float *samples, Rng &rng) {
  std::uniform_int_distribution<uint32_t> dist;
  uint32_t scramble = dist(rng);
  for (int i = 0; i < nSamples * nPixel; ++i)
    samples[i] = VanDerCorput(i, scramble);
  for (int i = 0; i < nPixel; ++i)
    Shuffle(samples + i * nSamples, nSamples, 1, rng);
  Shuffle(samples, nPixel, nSamples, rng);
}

template <typename Rng>
void LDShuffleScrambled2D(int nSamples, int nPixel, float *samples, Rng &rng) {
  uint32_t scramble[2] = {rng.RandomUInt(), rng.RandomUInt()};
  for (int i = 0; i < nSamples * nPixel; ++i)
    Sample02(i, scramble, &samples[2 * i]);
  for (int i = 0; i < nPixel; ++i)
    Shuffle(samples + 2 * i * nSamples, nSamples, 2, rng);
  Shuffle(samples, nPixel, 2 * nSamples, rng);
}

} // namespace pe
