#include "Memory\peAllocators.h"
#include "Algorithms/peAlgorithms.h"
#include "Memory\peMemoryUtil.h"

namespace pe {

//--------StackAllocator--------

peStackAllocator::peStackAllocator(size_t size, IAllocator *parentAllocator)
    : m_bufferSize(size), m_numAllocations(0), m_numFrees(0),
      m_sizePerAlloc(WrapAllocator<size_t>(parentAllocator)) {
  // Someone may pass nullptr as parent pointer
  if (parentAllocator == nullptr) {
    parentAllocator = GlobalAllocator;
  }
  m_parentAllocator = parentAllocator;
  m_buffer = (char *)parentAllocator->Allocate(size);
  m_topOfStack = m_buffer;
}

peStackAllocator::~peStackAllocator() {
  m_parentAllocator->Free(m_buffer);
  m_buffer = nullptr;
  m_topOfStack = nullptr;
  m_parentAllocator = nullptr;
  m_bufferSize = 0;
  m_numAllocations = 0;
  m_numFrees = 0;
}

void *peStackAllocator::Allocate(size_t numBytes) {
  // Check if sufficient memory!
  ptrdiff_t remaining = m_bufferSize - (m_topOfStack - m_buffer);
  PE_ASSERT(remaining >= 0);
  // Cast works because of Assert before!
  if (((size_t)remaining) < numBytes)
    return nullptr;
  char *stackPos = m_topOfStack;
  m_topOfStack += numBytes;
  m_numAllocations++;
  m_sizePerAlloc.push_back(numBytes);
  return stackPos;
}

void peStackAllocator::Free(void *mem) {
  if (mem == nullptr)
    return;
  size_t lastAllocSize = m_sizePerAlloc.back();
  PE_ASSERT(mem == (m_topOfStack - lastAllocSize));
  m_topOfStack = (char *)mem;
  m_numFrees++;
  m_sizePerAlloc.pop_back();
}

void peStackAllocator::Clear() {
  m_topOfStack = m_buffer;
  m_numAllocations = 0;
  m_numFrees = 0;
  m_sizePerAlloc.clear();
}

void *peStackAllocator::ObtainMarker() const { return m_topOfStack; }

void peStackAllocator::FreeToMarker(void *marker) {
  if (marker == nullptr)
    return;
  if (marker == m_topOfStack)
    return;
  PE_ASSERT(marker >= m_buffer && marker < m_topOfStack);
  size_t numFrees = 0;
  size_t curAllocPos = m_sizePerAlloc.size() - 1;
  do {
    size_t curAllocSize = m_sizePerAlloc[curAllocPos--];
    numFrees++;
    if (m_topOfStack - curAllocSize == marker) {
      m_topOfStack -= curAllocSize;
      for (size_t j = 0; j < numFrees; j++) {
        m_sizePerAlloc.pop_back();
      }
      m_numFrees += numFrees;
      break;
    }
    m_topOfStack -= curAllocSize;
  } while (curAllocPos != 0);
}

//----PoolAllocator----
pePoolAllocator::pePoolAllocator(size_t elementSize, size_t chunkSize,
                                 IAllocator *parentAllocator)
    : _parentAllocator(parentAllocator ? parentAllocator : GlobalAllocator),
      _chunks(WrapAllocator<char *>(_parentAllocator)), _chunkSize(chunkSize),
      _elementSize(elementSize),
      _freeList(WrapAllocator<FreeIndex>(parentAllocator)) {
  if (elementSize > chunkSize)
    throw std::runtime_error{"Element size must be less than chunk size!"};

  AllocateNewChunk();
}

pePoolAllocator::~pePoolAllocator() {
  for (auto chunk : _chunks)
    _parentAllocator->Free(chunk);
}

void *pePoolAllocator::Allocate(size_t numBytes) {
  if (numBytes > _elementSize)
    throw std::runtime_error{"Pool allocator can't allocate memory blocks that "
                             "are larger than its element size!"};
  if (_freeList.empty())
    AllocateNewChunk();
  // Take free indices from the back as this has better performance
  auto freeIndex = _freeList.back();
  _freeList.pop_back();

  return _chunks[freeIndex.chunkIdx] + freeIndex.offset;
}

void pePoolAllocator::Free(void *mem) {
  if (mem == nullptr)
    return;
  auto chunkIter =
      std::find_if(_chunks.begin(), _chunks.end(), [=](auto chunk) {
        auto distance = std::distance(chunk, reinterpret_cast<char *>(mem));
        return distance >= 0 &&
               distance <=
                   static_cast<int64_t>(_chunkSize -
                                        _elementSize); //-_elementSize because
                                                       // the last block in a
                                                       // chunk will be at this
                                                       // index!
      });
  if (chunkIter == _chunks.end())
    throw std::runtime_error{"Memory does not belong to this allocator!"};
  auto chunkIdx =
      static_cast<uint32_t>(std::distance(_chunks.begin(), chunkIter));
  auto offset = static_cast<uint32_t>(
      std::distance(*chunkIter, reinterpret_cast<char *>(mem)));
  _freeList.emplace_back(chunkIdx, offset);
}

pePoolAllocator::FreeIndex::FreeIndex(uint32_t chunkIdx, uint32_t offset)
    : chunkIdx(chunkIdx), offset(offset) {}

void pePoolAllocator::AllocateNewChunk() {
  auto chunk = reinterpret_cast<char *>(_parentAllocator->Allocate(_chunkSize));
  _chunks.push_back(chunk);
  auto chunkIdx = static_cast<uint32_t>(_chunks.size()) - 1;
  for (auto curElement = chunk;
       curElement < chunk + (_chunkSize - _elementSize);
       curElement += _elementSize) {
    auto offset = static_cast<uint32_t>(std::distance(chunk, curElement));
    _freeList.emplace_back(chunkIdx, offset);
  }
}
} // namespace pe
