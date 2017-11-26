#include "DataStructures\peWeakTable.h"

pe::peWeakHandle::peWeakHandle() { *this = Invalid(); }

pe::peWeakHandle::peWeakHandle(uint32_t index, uint32_t version)
    : index(index), version(version) {}

pe::peWeakHandle pe::peWeakHandle::Invalid() {
  auto uintMax = (std::numeric_limits<uint32_t>::max)();
  return {uintMax, uintMax};
}

pe::peWeakTableBase::peWeakTableBase(size_t elementSize, size_t chunkSize,
                                     IAllocator *parentAllocator)
    : _allocator(elementSize, chunkSize, parentAllocator) {}

void *pe::peWeakTableBase::Deref(peWeakHandle handle) const {
  auto iter = _entries.find(handle.index);
  if (iter == _entries.end())
    return nullptr;

  auto &entry = iter->second;
  if (entry.version != handle.version)
    return nullptr;

  return entry.instance;
}
