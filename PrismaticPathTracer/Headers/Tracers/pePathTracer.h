#pragma once
#include "Components/pePrimitiveRenderComponent.h"
#include "DataStructures/peVector.h"
#include "Sampling/peSampler.h"
#include "Scene/peScene.h"
#include "Threading/peTaskSystem.h"
#include "Type/peColor.h"

#include <atomic>
#include <mutex>
#include <optional>
#include <stdint.h>

namespace pe {
struct peCameraComponent;

//! \brief Path-tracing implementation
class pePathTracer {
public:
  using ImageData_t = peVector<RGBA_8Bit>;

  explicit pePathTracer(const peScene &scene);

  //! \brief Starts the (asynchronous) rendering process
  //! \param width Width of the image to render
  //! \param height Height of the image to render
  void BeginRenderProcess(uint32_t width, uint32_t height);

  //! \brief Returns true if a new result has arrived
  bool HasNewResult() const;

  //! \brief Stores the result image in the given buffer
  //! \param results Results buffer
  void GetResult(ImageData_t &results);

private:
  void GeneratePrimaryTasks(const peCameraComponent &camera);

  void TraceChunk(const peCameraComponent &camera, const glm::uvec2 &offset,
                  const glm::uvec2 &extent, uint32_t seed);

  void AccumulatePixels(gsl::span<RGBA_32BitFloat> newPixels,
                        const glm::uvec2 &offset, uint32_t stride);

  constexpr static uint32_t ChunkSizeX = 32;
  constexpr static uint32_t ChunkSizeY = 32;

  const peScene &_scene;

  uint32_t _width, _height;
  uint32_t _samplesPerPixel;
  Jitter _jitter;

  peTaskSystem _taskSystem;

  peVector<RGBA_32BitFloat> _pixelAccumulator;
  peVector<Spectrum_t> _normalizedPixels;
  mutable std::mutex _pixelsLock;
  std::atomic_bool _hasNewResult;
};

} // namespace pe
