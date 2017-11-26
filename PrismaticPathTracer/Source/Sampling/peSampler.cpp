#include "Sampling\peSampler.h"
#include "Components/peCameraComponent.h"
#include "Exceptions/peExceptions.h"
#include "Integration/peIntegrator.h"
#include "Math/MathUtil.h"
#include "Math/peSampling.h"
#include <chrono>

using namespace pe;

uint32_t Sample::Add1D(uint32_t numSamples) {
  _1dCounts.push_back(numSamples);
  return static_cast<uint32_t>(_1dCounts.size() - 1);
}

uint32_t Sample::Add2D(uint32_t numSamples) {
  _2dCounts.push_back(numSamples);
  return static_cast<uint32_t>(_2dCounts.size() - 1);
}

float *Sample::Get1D(uint32_t offset) const { return _1dSamples[offset]; }

float *Sample::Get2D(uint32_t offset) const { return _2dSamples[offset]; }

uint32_t Sample::Num1DChunks() const {
  return static_cast<uint32_t>(_1dCounts.size());
}

uint32_t Sample::Num2DChunks() const {
  return static_cast<uint32_t>(_2dCounts.size());
}

uint32_t Sample::ChunkSize1D(uint32_t chunkIdx) const {
  return _1dCounts[chunkIdx];
}

uint32_t Sample::ChunkSize2D(uint32_t chunkIdx) const {
  return _2dCounts[chunkIdx];
}

void Sample::CloneNTimes(gsl::span<Sample> clones) const {
  for (auto &clone : clones) {
    clone._1dCounts = _1dCounts;
    clone._2dCounts = _2dCounts;
    clone.AllocateSampleMemory();
  }
}

void Sample::AllocateSampleMemory() {
  const auto count2D =
      std::accumulate(_2dCounts.begin(), _2dCounts.end(), 0ull);
  const auto count1D =
      std::accumulate(_1dCounts.begin(), _1dCounts.end(), 0ull);
  const auto totalCount = count1D + 2 * count2D;

  _sampleBuffer.resize(totalCount);
  _1dSamples.resize(count1D);
  _2dSamples.resize(count2D);

  // Store pointers into '_sampleBuffer' for better cache locality
  auto bufferPtr = _sampleBuffer.data();
  for (size_t idx = 0; idx < count1D; ++idx) {
    _1dSamples[idx] = bufferPtr;
    bufferPtr += _1dCounts[idx];
  }
  for (size_t idx = 0; idx < count2D; ++idx) {
    _2dSamples[idx] = bufferPtr;
    bufferPtr += 2 * _2dCounts[idx];
  }
}

peSampler::peSampler(const glm::uvec2 &startPoint, const glm::uvec2 &endPoint,
                     uint32_t samplesX, uint32_t samplesY,
                     std::default_random_engine &rng)
    : _startPoint(startPoint), _endPoint(endPoint), _samplesX(samplesX),
      _samplesY(samplesY), _random(rng) {
  if (startPoint.x > endPoint.x || startPoint.y > endPoint.y)
    throw std::runtime_error{"Invalid start and end points!"};
}

uint32_t peSampler::MaxSampleCount() const { return _samplesX * _samplesY; }

peStratifiedSampler::peStratifiedSampler(const glm::uvec2 &startPoint,
                                         const glm::uvec2 &endPoint,
                                         uint32_t samplesX, uint32_t samplesY,
                                         std::default_random_engine &rng)
    : peSampler(startPoint, endPoint, samplesX, samplesY, rng) {
  _curX = startPoint.x;
  _curY = startPoint.y;
  _imageSamples.resize(samplesX * samplesY);
  _1dSamples.resize(samplesX * samplesY);
  _2dSamples.resize(samplesX * samplesY);
}

uint32_t peStratifiedSampler::GetMoreSamples(gsl::span<Sample> samples) {
  // TODO We can use different sampling strategies here, for the moment we will
  // stick with stratified sampling
  if (_curY == _endPoint.y)
    return 0;
  // Generate samples for a single pixel at a time
  const auto numSamples = _samplesX * _samplesY;
  if (samples.size() < numSamples)
    throw std::runtime_error{"Samples range is too small! Check "
                             "'MaxSamplesCount()' for the required size!"};
  StratifiedSample2D(_imageSamples, _samplesX, _samplesY, _random);

  const auto shift = glm::vec2{_curX, _curY};
  for (uint32_t idx = 0; idx < numSamples; ++idx) {
    auto &curSample = samples[idx];
    curSample.sampleValues = _imageSamples[idx] + shift;
    curSample.imagePosition = {_curX, _curY};
    // Generate 1D and 2D samples for integrators
    for (uint32_t chunkIdx = 0; chunkIdx < curSample.Num1DChunks();
         ++chunkIdx) {
      LatinHypercube(curSample.Get1D(chunkIdx),
                     curSample.Get1D(chunkIdx) +
                         curSample.ChunkSize1D(chunkIdx),
                     1, _random);
    }
    for (uint32_t chunkIdx = 0; chunkIdx < curSample.Num2DChunks();
         ++chunkIdx) {
      LatinHypercube(curSample.Get2D(chunkIdx),
                     curSample.Get2D(chunkIdx) +
                         2 * curSample.ChunkSize2D(chunkIdx),
                     2, _random);
    }
  }

  ++_curX;
  if (_curX == _endPoint.x) {
    _curX = _startPoint.x;
    ++_curY;
  }

  return numSamples;
}

Sample::Sample(peSurfaceIntegrator &surfaceIntegrator, const peScene &scene) {
  surfaceIntegrator.AllocateSamples(*this, scene);
}

BSDFSampleOffset::BSDFSampleOffset(uint32_t numSamples, Sample &sample)
    : numSamples(numSamples) {
  directionOffset = sample.Add2D(numSamples);
  componentOffset = sample.Add1D(numSamples);
}

BSDFSample BSDFSampleOffset::ToBSDFSample(const Sample &sample,
                                          uint32_t sampleIndex) const {
  const auto buffer1D = sample.Get1D(componentOffset);
  const auto buffer2D = sample.Get2D(directionOffset);
  return BSDFSample{{buffer2D[2 * sampleIndex], buffer2D[2 * sampleIndex + 1]},
                    buffer1D[sampleIndex]};
}
