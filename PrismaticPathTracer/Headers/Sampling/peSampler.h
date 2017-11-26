#pragma once
#include "Util/Ray.h"

#include "DataStructures/peVector.h"
#include "peLightSampler.h"
#include <random>
#include <span.h>

namespace pe {
struct peSurfaceIntegrator;

class peBRDF;

//! \brief Different jitter strategies for generating samples
enum class Jitter {
  //! \brief No jitter, always use fixed pixel values
  None,
  //! \brief Uniform jitter in a [-0.5;0.5]² interval around the pixel position
  Uniform,
  //! \brief Gaussian jitter in a [-0.5;0.5]² interval around the pixel position
  Gauss
};

//! \brief Sample from a sampler
struct Sample {
  Sample() = default;
  Sample(peSurfaceIntegrator &surfaceIntegrator, const peScene &scene);
  //! \brief Requests 'numSamples' 1D samples and returns the offset into this
  //! sample at which the requested samples can be accessed
  //! \param numSamples Number of samples
  //! \returns Offset to samples
  uint32_t Add1D(uint32_t numSamples);

  //! \brief Requests 'numSamples' 2D samples and returns the offset into this
  //! sample at which the requested samples can be accessed
  //! \param numSamples Number of samples
  //! \returns Offset to samples
  uint32_t Add2D(uint32_t numSamples);

  //! \brief Returns the 1D samples chunk at the given offset
  //! \param offset Offset
  //! \returns Pointer to the 1D samples chunk at offset
  float *Get1D(uint32_t offset) const;
  //! \brief Returns the 2D samples chunk at the given offset
  //! \param offset Offset
  //! \returns Pointer to the 2D samples chunk at offset
  float *Get2D(uint32_t offset) const;

  //! \brief Returns the number of 1D sample chunks
  //! \returns Number of 1D sample chunks
  uint32_t Num1DChunks() const;
  //! \brief Returns the number of 2D sample chunks
  //! \returns Number of 2D sample chunks
  uint32_t Num2DChunks() const;

  //! \brief Returns the number of 1D samples in the given chunk
  //! \param chunkIdx Index of a chunk
  //! \returns Number of samples in chunk
  uint32_t ChunkSize1D(uint32_t chunkIdx) const;
  //! \brief Returns the number of 2D samples in the given chunk
  //! \param chunkIdx Index of a chunk
  //! \returns Number of samples in chunk
  uint32_t ChunkSize2D(uint32_t chunkIdx) const;

  //! \brief Clones this sample multiple times
  void CloneNTimes(gsl::span<Sample> clones) const;

  glm::vec2 sampleValues;
  glm::uvec2 imagePosition;

private:
  void AllocateSampleMemory();

  //! \brief Stores the number of samples for each sample block requested
  //! through 'Add1D'
  peVector<uint32_t> _1dCounts;
  //! \brief Stores the number of samples for each sample block requested
  //! through 'Add2D'
  peVector<uint32_t> _2dCounts;
  peVector<float> _sampleBuffer;
  peVector<float *> _1dSamples;
  peVector<float *> _2dSamples;
};

//! \brief Creates random samples
class peSampler {
public:
  peSampler(const glm::uvec2 &startPoint, const glm::uvec2 &endPoint,
            uint32_t samplesX, uint32_t samplesY,
            std::default_random_engine &rng);

  virtual ~peSampler() = default;

  virtual uint32_t GetMoreSamples(gsl::span<Sample> samples) = 0;

  uint32_t MaxSampleCount() const;

protected:
  const glm::uvec2 _startPoint, _endPoint;
  const uint32_t _samplesX, _samplesY;
  std::default_random_engine &_random;
};

//! \brief Sampler that uses stratified sampling
class peStratifiedSampler : public peSampler {
public:
  peStratifiedSampler(const glm::uvec2 &startPoint, const glm::uvec2 &endPoint,
                      uint32_t samplesX, uint32_t samplesY,
                      std::default_random_engine &rng);

  uint32_t GetMoreSamples(gsl::span<Sample> samples) override;

private:
  uint32_t _curX, _curY;
  peVector<glm::vec2> _imageSamples;
  peVector<float> _1dSamples;
  peVector<glm::vec2> _2dSamples;
};

//! \brief Offsets into a samples buffer for BSDFSamples
struct BSDFSampleOffset {
  BSDFSampleOffset() = default;
  BSDFSampleOffset(uint32_t numSamples, Sample &sample);

  BSDFSample ToBSDFSample(const Sample &sample, uint32_t sampleIndex) const;

  uint32_t numSamples, directionOffset, componentOffset;
};

} // namespace pe
