#pragma once
#include "peAllocatable.h"
#include "peMemoryUtil.h"
#include "peUtilDefs.h"
#include <map>

#include <Windows.h>

namespace pe {

class PE_UTIL_API IAllocator {
public:
  virtual ~IAllocator() {}

  virtual void *Allocate(size_t numBytes) = 0;
  virtual void Free(void *mem) = 0;
};

class PE_UTIL_API IAllocatorStatistics : public IAllocator {
public:
  virtual size_t GetReservedMemory() const = 0;
  virtual size_t GetFreeMemory() const = 0;
  virtual size_t GetTotalMemory() const = 0;
  virtual size_t GetNumAllocations() const = 0;
  virtual size_t GetNumFrees() const = 0;

  virtual IAllocator *GetParentAllocator() const = 0;
};

class PE_UTIL_API peStdAllocator : public IAllocatorStatistics {
public:
  static peStdAllocator *GetInstance();

  // Getters
  size_t GetFreeMemory() const override {
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
  }
  size_t GetNumAllocations() const override { return m_numAllocations; }
  size_t GetNumFrees() const override { return m_numFrees; }
  IAllocator *GetParentAllocator() const override { return nullptr; }
  size_t GetReservedMemory() const override { return m_reservedMemory; }
  size_t GetTotalMemory() const override {
    return GetFreeMemory() + GetReservedMemory();
  }

  // Allocation/deallocation
  void *Allocate(size_t numBytes) override;
  void Free(void *mem) override;

private:
  size_t m_numAllocations;
  size_t m_numFrees;
  size_t m_reservedMemory;

  peStdAllocator();
  ~peStdAllocator();
};

class PE_UTIL_API peLeakDetectionAllocator : public IAllocatorStatistics {
public:
  virtual void *Allocate(size_t size) override;
  virtual void Free(void *mem) override;

  static IAllocatorStatistics *GetInstance();

  // Getters
  virtual size_t GetFreeMemory() const override {
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
  }
  virtual size_t GetNumAllocations() const override { return m_numAllocations; }
  virtual size_t GetNumFrees() const override { return m_numFrees; }
  virtual IAllocator *GetParentAllocator() const override { return nullptr; }
  virtual size_t GetReservedMemory() const override { return m_reservedMemory; }
  virtual size_t GetTotalMemory() const override {
    return GetFreeMemory() + GetReservedMemory();
  }

  void LogLeaks();

private:
#define MAX_ALLOCATIONS 10000
  struct AllocationInfo {
    size_t size;
    void *mem;
    size_t backtrace[20];
    uint8_t numTraceFrames;
  };

  static size_t MemHash(void *mem);

  size_t m_numAllocations;
  size_t m_numFrees;
  size_t m_reservedMemory;

  // std::map<void*, AllocationInfo> m_allocations;
  AllocationInfo m_allocations[MAX_ALLOCATIONS];

  peLeakDetectionAllocator();
  virtual ~peLeakDetectionAllocator();

  void TrackAllocation(size_t size, void *mem);
  void TrackFree(void *mem);
};

#pragma region NewDelete

template <typename T, typename... Args>
std::enable_if_t<std::is_base_of<pe::peAllocatable, T>::value, T *>
New(Args &&... ctorArgs, IAllocator *allocator) {
  T *ret =
      new (allocator->Allocate(sizeof(T))) T(std::forward<Args>(ctorArgs)...);
  ret->SetAllocator(allocator);
  return ret;
}

template <typename T>
std::enable_if_t<std::is_base_of<pe::peAllocatable, T>::value, T *>
New(IAllocator *allocator) {
  auto ret = new (allocator->Allocate(sizeof(T))) T();
  ret->SetAllocator(allocator);
  return ret;
}

template <typename T, typename... Args>
std::enable_if_t<!std::is_base_of<pe::peAllocatable, T>::value, T *>
New(Args &&... ctorArgs, IAllocator *allocator) {
  return new (allocator->Allocate(sizeof(T)))
      T(std::forward<Args>(ctorArgs)...);
}

template <typename T>
std::enable_if_t<!std::is_base_of<pe::peAllocatable, T>::value, T *>
New(IAllocator *allocator) {
  return new (allocator->Allocate(sizeof(T))) T();
}

template <typename T>
std::enable_if_t<std::is_base_of<pe::peAllocatable, T>::value> Delete(T *obj) {
  auto allocator = obj->GetAllocator();
  obj->~T();
  allocator->Free(static_cast<void *>(obj));
}

template <typename T> void Delete(T *obj, IAllocator *allocator) {
  MemoryUtil::Destroy(obj, 1);
  allocator->Free(static_cast<void *>(obj));
}

template <typename T> void DeleteAndNull(T *&obj) {
  Delete(obj);
  obj = nullptr;
}

template <typename T> void DeleteAndNull(T *&obj, IAllocator *allocator) {
  Delete(obj, allocator);
  obj = nullptr;
}

#pragma endregion

} // namespace pe

#define TRACK_LEAKS

#ifdef TRACK_LEAKS
#define GlobalAllocator pe::peLeakDetectionAllocator::GetInstance()
#define PrintLeaks                                                             \
  static_cast<rtr::LeakDetectionAllocator &>(                                  \
      rtr::LeakDetectionAllocator::GetInstance())                              \
      .LogLeaks();
#else
#define GlobalAllocator pe::peStdAllocator::GetInstance()
#define PrintLeaks
#endif

#include <new>

void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *mem);
void operator delete[](void *mem);
