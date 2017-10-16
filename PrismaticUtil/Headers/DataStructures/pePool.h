#pragma once
#include "Memory/peAllocators.h"
#include "peVector.h"

namespace pe {

class peBasePool {
public:
  explicit peBasePool(size_t elementSize, size_t chunkSize = 8192,
                      IAllocator *parentAllocator = GlobalAllocator)
      : _allocator(parentAllocator), _chunkSize(chunkSize),
        _elementSize(elementSize) {
    if (elementSize > chunkSize)
      throw std::runtime_error{"Element size must not exceed chunk size!"};
  }

  //! \brief Reserve at least 'count' elements
  //! \param count Number of elements to reserve
  void Reserve(size_t count) {
    auto diff = static_cast<int64_t>(count) - static_cast<int64_t>(GetCount());
    if (diff <= 0)
      return;
    const auto elementsPerChunk = _chunkSize / ElementSize();
    auto chunksToAdd = static_cast<size_t>(diff) / elementsPerChunk;
    AddChunks(chunksToAdd);
  }

  size_t GetCount() const {
    const auto elementsPerChunk = _chunkSize / ElementSize();
    return _chunks.size() * elementsPerChunk;
  }

  size_t ElementSize() const { return _elementSize; }

  void *Get(size_t idx) const {
    if (idx >= GetCount())
      throw std::runtime_error{"Index out of bounds!"};
    auto chunkIdx = idx / _chunkSize;
    auto offset = idx % _chunkSize;
    return _chunks[chunkIdx] + offset;
  }

  virtual void DestructAt(size_t idx) = 0;

private:
  void AddChunks(size_t numChunks) {
    for (size_t idx = 0; idx < numChunks; ++idx) {
      _chunks.push_back(
          reinterpret_cast<Chunk_t>(_allocator->Allocate(_chunkSize)));
    }
  }

  using Chunk_t = char *;
  IAllocator *const _allocator;
  const size_t _chunkSize;
  const size_t _elementSize;

  peVector<Chunk_t> _chunks;
};

template <typename T> class pePool : public peBasePool {
public:
  explicit pePool(size_t chunkSize = 8192,
                  IAllocator *parentAllocator = GlobalAllocator)
      : peBasePool(sizeof(T), chunkSize, parentAllocator) {}

  void DestructAt(size_t idx) override {
    auto obj = reinterpret_cast<T *>(Get(idx));
    obj->~T();
  }
};

} // namespace pe
