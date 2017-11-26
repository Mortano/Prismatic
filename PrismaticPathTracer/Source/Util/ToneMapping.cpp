#include "Util\ToneMapping.h"
#include "Type\peColor.h"

static uint8_t Saturate(float val) {
  return static_cast<uint8_t>(std::min(255, static_cast<int>(val * 255)));
}

static void ToneMap_Saturate(gsl::span<pe::RGBA_8Bit> result,
                             const gsl::span<pe::Spectrum_t> input,
                             const uint32_t imageWidth,
                             const uint32_t imageHeight) {
  for (size_t idx = 0; idx < input.size(); ++idx) {
    auto &in = input[idx];
    result[idx] = {Saturate(in.r()), Saturate(in.g()), Saturate(in.b()), 255};
  }
}

static void ToneMap_Linear(gsl::span<pe::RGBA_8Bit> result,
                           const gsl::span<pe::Spectrum_t> input,
                           const uint32_t imageWidth,
                           const uint32_t imageHeight) {
  float maxR = 0.f, maxG = 0.f, maxB = 0.f;
  for (auto &pixel : input) {
    maxR = std::max(maxR, pixel.r());
    maxG = std::max(maxG, pixel.g());
    maxB = std::max(maxB, pixel.b());
  }

  if (maxR == 0.f)
    maxR = 1.f;
  else
    maxR = 1.f / maxR;
  if (maxG == 0.f)
    maxG = 1.f;
  else
    maxG = 1.f / maxG;
  if (maxB == 0.f)
    maxB = 1.f;
  else
    maxB = 1.f / maxB;

  for (size_t idx = 0; idx < input.size(); ++idx) {
    auto &in = input[idx];
    result[idx] = {Saturate(in.r() * maxR), Saturate(in.g() * maxG),
                   Saturate(in.b() * maxB), 255};
  }
}

void pe::ToneMap(gsl::span<RGBA_8Bit> result, const gsl::span<Spectrum_t> input,
                 const uint32_t imageWidth, const uint32_t imageHeight,
                 const ToneMapping strategy) {
  if (input.size() != result.size())
    throw std::runtime_error{"Input and output image sizes must match!"};
  switch (strategy) {
  case ToneMapping::Saturate:
    ToneMap_Saturate(result, input, imageWidth, imageHeight);
    break;
  case ToneMapping::Linear:
    ToneMap_Linear(result, input, imageWidth, imageHeight);
    break;
  case ToneMapping::Log:
    break;
  default:
    break;
  }
}
