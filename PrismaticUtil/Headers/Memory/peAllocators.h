#pragma once
#include "DataStructures/peVector.h"
#include "peAllocator.h"
#include "peUtilDefs.h"

namespace pe {

#pragma warning(push)
#pragma warning(disable : 4251)

//! Simple stack allocator to allocate continous memory
class PE_UTIL_API peStackAllocator : public IAllocatorStatistics {
public:
  explicit peStackAllocator(
      size_t size, IAllocator *parentAllocator = peStdAllocator::GetInstance());
  ~peStackAllocator();

  peStackAllocator(const peStackAllocator &) = delete;
  const peStackAllocator &operator=(const peStackAllocator &) = delete;

  // Getters
  size_t GetFreeMemory() const override {
    return m_bufferSize - GetReservedMemory();
  }

  size_t GetNumAllocations() const override { return m_numAllocations; }
  size_t GetNumFrees() const override { return m_numFrees; }
  IAllocator *GetParentAllocator() const override { return m_parentAllocator; }

  size_t GetReservedMemory() const override {
    auto memDiff = (m_topOfStack - m_buffer);
    return memDiff;
  }

  size_t GetTotalMemory() const override { return m_bufferSize; }

  void *Allocate(size_t numBytes) override;
  void Free(void *mem) override;

  //! Clears the stack and resets the number of allocations / frees
  void Clear();
  //! Obtain a marker to the current stack position
  void *ObtainMarker() const;
  //! Free the stack to the given position
  void FreeToMarker(void *marker);

private:
  IAllocator *m_parentAllocator;

  char *m_buffer;
  char *m_topOfStack;
  size_t m_bufferSize;
  size_t m_numAllocations;
  size_t m_numFrees;
  peVector<size_t> m_sizePerAlloc;
};

//! Simple pool allocator to allocate fixed size memeory blocks
class PE_UTIL_API pePoolAllocator : public IAllocator {
public:
  /// <summary>
  /// Creates a new pool allocator
  /// </summary>
  /// <param name="elementSize">Size of each element in the pool</param>
  /// <param name="chunkSize">Size of each new memory block that will be
  /// allocated whenever there are no free elements</param> <param
  /// name="parentAllocator">Parent allocator</param>
  explicit pePoolAllocator(
      size_t elementSize, size_t chunkSize = 8192,
      IAllocator *parentAllocator = peStdAllocator::GetInstance());
  ~pePoolAllocator();

  pePoolAllocator(const pePoolAllocator &) = delete;
  const pePoolAllocator &operator=(const pePoolAllocator &) = delete;

  // Allocation/deallocation
  void *Allocate(size_t numBytes) override;
  void Free(void *mem) override;

  const auto &GetChunks() const { return _chunks; }

private:
  struct FreeIndex {
    FreeIndex(uint32_t, uint32_t);
    const uint32_t chunkIdx;
    const uint32_t offset;
  };

  void AllocateNewChunk();

  IAllocator *_parentAllocator;

  peVector<char *> _chunks;
  const size_t _chunkSize;
  const size_t _elementSize;

  peVector<FreeIndex> _freeList;
  static constexpr int INVALID_IDX = -1;
};

#pragma warning(pop)

} // namespace pe
