#include "Util\Ray.h"

#include "Components\peCameraComponent.h"
#include "Math\MathUtil.h"
#include "Sampling\peSampler.h"

static pe::Ray CalcPrimaryRay(const pe::peCameraComponent &camera,
                              const glm::vec2 &xy,
                              const glm::uvec2 &screenSize) {
  const auto tanFov = std::tan(camera.fov / 2);
  const auto aspect = screenSize.x / static_cast<float>(screenSize.y);

  const auto cx = aspect * ((xy.x * 2.f / screenSize.x) - 1.f) * tanFov;
  const auto cy = ((xy.y * 2.f / screenSize.y) - 1.f) * tanFov;

  const auto fwd = pe::ToVec3(pe::Forward(camera.view));
  const auto right = pe::ToVec3(pe::Right(camera.view));
  const auto up = pe::ToVec3(pe::Up(camera.view));
  const auto pos = pe::ToVec3(pe::Position(camera.view));

  pe::Ray ret;
  ret.origin = pos + fwd + right * cx + up * cy;
  ret.direction = glm::normalize(ret.origin - pos);
  ret.t = camera.zMax;
  return ret;
}

pe::Ray::Ray() : origin(0, 0, 0), direction(0, 0, 0), t(0) {}

pe::Ray::Ray(const glm::vec3 &origin, const glm::vec3 &dir, float t)
    : origin(origin), direction(dir), t(t) {}

void pe::GetPrimaryRaysFromSamples(gsl::span<Ray> rays,
                                   gsl::span<Sample> samples,
                                   const peCameraComponent &camera,
                                   const glm::uvec2 &screenSize) {
  if (rays.size() < samples.size())
    throw std::runtime_error{
        "Rays range must be at least as big as the samples range!"};
  std::transform(samples.begin(), samples.end(), rays.begin(),
                 [&](const pe::Sample &sample) {
                   return CalcPrimaryRay(camera, sample.sampleValues,
                                         screenSize);
                 });
}
