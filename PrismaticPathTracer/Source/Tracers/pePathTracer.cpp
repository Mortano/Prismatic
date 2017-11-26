#include "Tracers\pePathTracer.h"
#include "Components/peCameraComponent.h"
#include "Integration/peDebugIntegrator.h"
#include "Integration/peDirectLightIntegrator.h"
#include "Integration/pePathTracingIntegrator.h"
#include "Math/MathUtil.h"
#include "Math/peCoordSys.h"
#include "Sampling/peSampler.h"
#include "Util/Intersections.h"
#include "Util/Ray.h"
#include "Util\ToneMapping.h"
#include "peEngine.h"

//#define LOG_HITS

static pe::peComponentHandle<pe::peCameraComponent> GetActiveCamera() {
  auto cameras = PrismaticEngine.GetWorld()
                     ->EntityManager()
                     .AllComponents<pe::peCameraComponent>();
  pe::peComponentHandle<pe::peCameraComponent> camera;
  for (auto cam : cameras) {
    camera = cam;
    break;
  }

  return camera;
}

pe::pePathTracer::pePathTracer(const peScene &scene)
    : _scene(scene), _width(0), _height(0), _samplesPerPixel(16),
      _jitter(Jitter::Uniform) {}

void pe::pePathTracer::BeginRenderProcess(uint32_t width, uint32_t height) {
  _taskSystem.Start();

  auto camera = GetActiveCamera();
  if (!camera)
    return;

  _width = width;
  _height = height;
  _pixelAccumulator.resize(width * height, RGBA_32BitFloat{0, 0, 0, 0});
  _normalizedPixels.resize(width * height, Spectrum_t{0, 0, 0});

  GeneratePrimaryTasks(*camera);
}

bool pe::pePathTracer::HasNewResult() const {
  return _hasNewResult.load(std::memory_order::memory_order_acquire);
}

void pe::pePathTracer::GetResult(ImageData_t &results) {
  // Resolve the accumulation buffer into the resolved image
  results.resize(_pixelAccumulator.size());
  {
    std::lock_guard<std::mutex> guard{_pixelsLock};

    // First normalize all the pixels since we might have multiple samples per
    // pixel
    std::transform(
        _pixelAccumulator.begin(), _pixelAccumulator.end(),
        _normalizedPixels.begin(), [](auto &px) {
          auto div = px.a() > 0 ? (1 / px.a()) : 1;
          return Spectrum_t{px.r() * div, px.g() * div, px.b() * div};
        });

    // then tone map
    ToneMap(results, _normalizedPixels, _width, _height, ToneMapping::Saturate);

    _hasNewResult = false;
  }
}

void pe::pePathTracer::GeneratePrimaryTasks(const peCameraComponent &camera) {
  const auto chunksX = (_width + ChunkSizeX - 1) / ChunkSizeX;
  const auto chunksY = (_height + ChunkSizeY - 1) / ChunkSizeY;

  std::default_random_engine rng;
  auto now = std::chrono::high_resolution_clock::now();
  rng.seed(static_cast<uint32_t>(now.time_since_epoch().count()));

  std::vector<uint32_t> seeds;
  seeds.resize(chunksX * chunksY);
  std::generate(seeds.begin(), seeds.end(), [&]() { return rng(); });

#ifdef _DEBUG
  _taskSystem.AddTask([&]() {
    TraceChunk(camera, {0, 0}, {_width, _height}, seeds[0]);
  });
#else
  for (uint32_t y = chunksY; y > 0; --y) {
    for (uint32_t x = 0; x < chunksX; ++x) {
      const auto offsetX = x * ChunkSizeX;
      const auto offsetY = (y - 1) * ChunkSizeY;

      auto seed = seeds[(y * chunksX) + x];

      _taskSystem.AddTask([&, offsetX, offsetY, seed]() {
        TraceChunk(camera, {offsetX, offsetY}, {ChunkSizeX, ChunkSizeY}, seed);
      });
    }
  }
#endif
}

void pe::pePathTracer::TraceChunk(const peCameraComponent &camera,
                                  const glm::uvec2 &offset,
                                  const glm::uvec2 &extent, uint32_t seed) {
  const auto rangeXEnd = std::min(offset.x + extent.x, _width);
  const auto rangeYEnd = std::min(offset.y + extent.y, _height);
  auto sqrtSamples = static_cast<uint32_t>(std::sqrt(_samplesPerPixel));

  std::default_random_engine rng;
  rng.seed(seed);

  peStratifiedSampler sampler{{offset.x, offset.y},
                              {rangeXEnd, rangeYEnd},
                              sqrtSamples,
                              sqrtSamples,
                              rng};

  // peDirectLightIntegrator integrator;
  pePathTracingIntegrator integrator{5};
  // peDebugIntegrator integrator;
  // integrator.SetVisualizationMode(DebugVisualizationMode::Sample);
  Sample baseSample{integrator, _scene};

  peVector<Sample> samples;
  samples.resize(sampler.MaxSampleCount());
  baseSample.CloneNTimes(samples);

  peVector<Ray> rays;
  rays.resize(sampler.MaxSampleCount());

  peVector<RGBA_32BitFloat> colorAccumulator;
  colorAccumulator.resize(extent.x * extent.y, RGBA_32BitFloat{0, 0, 0, 0});

  uint32_t totalSamplesProcessed = 0;
  constexpr uint32_t UpdateAfterNSamples = 2048;

  auto camFwd = ToVec3(Forward(camera.view));

  uint32_t numGeneratedSamples;
  while ((numGeneratedSamples = sampler.GetMoreSamples(samples)) != 0) {
    GetPrimaryRaysFromSamples(rays, samples, camera, {_width, _height});

    for (uint32_t idx = 0; idx < numGeneratedSamples; ++idx) {
      auto &ray = rays[idx];
      const auto hit = _scene.GetIntersection(ray);

#ifdef LOG_HITS
      PrismaticEngine.GetLogging()->LogInfo(
          "Tracing ray [origin: (%f;%f;%f) dir: (%f;%f;%f)] for screen "
          "position (%u;%u)",
          ray.origin.x, ray.origin.y, ray.origin.z, ray.direction.x,
          ray.direction.y, ray.direction.z, samples[idx].imagePosition.x,
          samples[idx].imagePosition.y);
#endif

      if (!hit)
        continue;

      auto &bsdf = _scene.GetBSDFForPrimitive(hit->primitiveID);

      auto &sample = samples[idx];
      auto radiance = integrator.Estimate(
          _scene, sample, *hit, ray.direction * -1.f, bsdf, BxDFType::All, rng);

      if (std::isnan(radiance.r()) || std::isnan(radiance.g()) ||
          std::isnan(radiance.b())) {
        __debugbreak();
      }

#ifdef LOG_HITS
      PrismaticEngine.GetLogging()->LogInfo("Registered hit at [%u;%u]",
                                            sample.imagePosition.x,
                                            sample.imagePosition.y);
#endif

      auto &col =
          colorAccumulator[(sample.imagePosition.y - offset.y) * extent.x +
                           (sample.imagePosition.x - offset.x)];

      col += RGBA_32BitFloat{radiance.r(), radiance.g(), radiance.b(), 1.f};
    }

    totalSamplesProcessed += numGeneratedSamples;
    if (totalSamplesProcessed >= UpdateAfterNSamples) {
      totalSamplesProcessed -= UpdateAfterNSamples;
      AccumulatePixels(colorAccumulator, {offset.x, offset.y}, extent.x);
    }
  }

  AccumulatePixels(colorAccumulator, {offset.x, offset.y}, extent.x);
}

void pe::pePathTracer::AccumulatePixels(gsl::span<RGBA_32BitFloat> newPixels,
                                        const glm::uvec2 &offset,
                                        uint32_t stride) {
  std::lock_guard<std::mutex> guard{_pixelsLock};

  const auto rows = static_cast<uint32_t>(newPixels.size() / stride);
  const auto end = std::min(offset.y + rows, _height);
  for (auto y = offset.y; y < end; ++y) {
    // The newPixels buffer might extend past the end of the _pixelAccumulator,
    // so we have to make sure  that we don't go out of bounds!
    const auto dstStartIdx = (y * _width) + offset.x;
    const auto dstEndIdx =
        std::min(dstStartIdx + stride,
                 static_cast<uint32_t>(_pixelAccumulator.size()) - 1);

    const auto srcStartIdx = (y - offset.y) * stride;
    const auto srcEndIdx = srcStartIdx + (dstEndIdx - dstStartIdx);

    auto srcStart = newPixels.begin() + srcStartIdx;
    const auto srcEnd = newPixels.begin() + srcEndIdx;
    auto dstStart = _pixelAccumulator.begin() + dstStartIdx;

    for (; srcStart != srcEnd; ++srcStart, ++dstStart) {
      *dstStart = *srcStart;
    }
  }

  _hasNewResult = true;
}
