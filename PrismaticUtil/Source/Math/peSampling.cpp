#include "Math\peSampling.h"
#include <limits>

constexpr static float OneMinusEpsilon =
    1.f - std::numeric_limits<float>::epsilon();

glm::vec3 pe::UniformSampleHemisphere(const float rnd1, const float rnd2) {
  const auto z = rnd1;
  const auto r = std::sqrtf(std::max(0.f, 1.f - z * z));
  const auto phi = 2 * Pi<float> * rnd2;
  const auto x = r * std::cosf(phi);
  const auto y = r * std::sinf(phi);
  return {x, y, z};
}

float pe::UniformSampleHemispherePdf() { return 1.f / (2 * Pi<float>); }

glm::vec3 pe::UniformSampleSphere(const float rnd1, const float rnd2) {
  const auto z = 1.f - (2.f * rnd1);
  const auto r = std::sqrtf(std::max(0.f, 1.f - z * z));
  const auto phi = 2.f * Pi<float> * rnd2;
  const auto x = r * std::cosf(phi);
  const auto y = r * std::sinf(phi);
  return {x, y, z};
}

float pe::UniformSampleSpherePdf() { return 1.f / (4.f * Pi<float>); }

glm::vec2 pe::ConcentricSampleDisk(const float rnd1, const float rnd2) {
  if (rnd1 == 0.f && rnd2 == 0.f)
    return {0.f, 0.f};
  auto rTheta = [](const float sx, const float sy) -> std::pair<float, float> {
    if (sx >= -sy) {
      if (sx > sy) {
        // First region of disk
        return sy > 0.f ? std::make_pair(sx, sy / sx)
                        : std::make_pair(sx, 8.f + sy / sx);
      } else {
        // Second region of disk
        return std::make_pair(sy, 2.f - sx / sy);
      }
    } else {
      if (sx <= sy) {
        // Third region of disk
        return std::make_pair(-sx, 4.f - sy / sx);
      } else {
        // Fourth region of disk
        return std::make_pair(-sy, 6.f + sx / sy);
      }
    }
  }(2 * rnd1 - 1, 2 * rnd2 - 1);
  const auto &r = rTheta.first;
  const auto theta = rTheta.second * Pi<float> / 4.f;
  return {r * std::cosf(theta), r * std::sinf(theta)};
}

glm::vec3 pe::CosineSampleHemisphere(const float rnd1, const float rnd2) {
  const auto rndDisk = ConcentricSampleDisk(rnd1, rnd2);
  const auto z = std::sqrtf(
      std::max(0.f, 1.f - rndDisk.x * rndDisk.x - rndDisk.y * rndDisk.y));
  return {rndDisk.x, rndDisk.y, z};
}

float pe::CosineSampleHemispherePdf(float cosTheta, float phi) {
  return cosTheta * InvPi<float>;
}

float pe::PowerHeuristic(int nf, float fPdf, int ng, float gPdf) {
  auto f = nf * fPdf, g = ng * gPdf;
  return (f * f) / (f * f + g * g);
}

void pe::Sample02(uint32_t n, const uint32_t scramble[2], float sample[2]) {
  sample[0] = VanDerCorput(n, scramble[0]);
  sample[1] = Sobol2(n, scramble[1]);
}

float pe::VanDerCorput(uint32_t n, uint32_t scramble) {
  // Reverse bits of _n_
  n = (n << 16) | (n >> 16);
  n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
  n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
  n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
  n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
  n ^= scramble;
  return std::min(((n >> 8) & 0xffffff) / float(1 << 24), OneMinusEpsilon);
}

float pe::Sobol2(uint32_t n, uint32_t scramble) {
  for (uint32_t v = 1 << 31; n != 0; n >>= 1, v ^= v >> 1)
    if (n & 0x1)
      scramble ^= v;
  return std::min(((scramble >> 8) & 0xffffff) / float(1 << 24),
                  OneMinusEpsilon);
}
