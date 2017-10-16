#pragma once
#include "peAllocator.h"
#include <vector>

namespace pe {

//! \brief STL allocator that wraps the IAllocator of this engine
template <typename T> class peStlAllocatorWrapper {
public:
  template <typename U> friend class peStlAllocatorWrapper;

  using pointer = T *;
  using const_pointer = const T *;
  using void_pointer = void *;
  using const_void_pointer = const void *;

  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap = std::true_type;

  template <typename U> struct rebind {
    using other = peStlAllocatorWrapper<U>;
  };

  peStlAllocatorWrapper() : _allocator(GlobalAllocator) {}

  explicit peStlAllocatorWrapper(IAllocator *allocator)
      : _allocator(allocator) {}

  ~peStlAllocatorWrapper() = default;

  peStlAllocatorWrapper(const peStlAllocatorWrapper &other)
      : _allocator(other._allocator) {}

  peStlAllocatorWrapper(peStlAllocatorWrapper &&other) noexcept
      : _allocator(other._allocator) {
    other._allocator = nullptr;
  }

  template <typename U, typename = std::enable_if_t<!std::is_same_v<U, T>>>
  peStlAllocatorWrapper(const peStlAllocatorWrapper<U> &other)
      : _allocator(other._allocator) {}

  template <typename U, typename = std::enable_if_t<!std::is_same_v<U, T>>>
  peStlAllocatorWrapper(peStlAllocatorWrapper<U> &&other) noexcept
      : _allocator(other._allocator) {
    other._allocator = nullptr;
  }

  peStlAllocatorWrapper &operator=(const peStlAllocatorWrapper &other) {
    _allocator = other._allocator;
    return *this;
  }

  peStlAllocatorWrapper &operator=(peStlAllocatorWrapper &&other) noexcept {
    _allocator = other._allocator;
    other._allocator = nullptr;
    return *this;
  }

  template <typename U, typename = std::enable_if_t<!std::is_same_v<U, T>>>
  peStlAllocatorWrapper &operator=(const peStlAllocatorWrapper<U> &other) {
    _allocator = other._allocator;
    return *this;
  }

  template <typename U, typename = std::enable_if_t<!std::is_same_v<U, T>>>
  peStlAllocatorWrapper &operator=(peStlAllocatorWrapper<U> &&other) noexcept {
    _allocator = other._allocator;
    other._allocator = nullptr;
    return *this;
  }

  pointer allocate(size_t n) {
    return static_cast<pointer>(_allocator->Allocate(n * sizeof(T)));
  }

  void deallocate(pointer ptr, size_t n) {
    _allocator->Free(reinterpret_cast<void *>(ptr));
  }

  auto select_on_container_copy_construction() const { return *this; }

  bool operator==(const peStlAllocatorWrapper &other) const {
    return _allocator == other._allocator;
  }

  bool operator!=(const peStlAllocatorWrapper &other) const {
    return !operator==(other);
  }

private:
  IAllocator *_allocator;
};

//! \brief Wraps an allocator to be used with STL containers
template <typename T>
peStlAllocatorWrapper<T> WrapAllocator(IAllocator *allocator) {
  return peStlAllocatorWrapper<T>(allocator);
}

} // namespace pe
