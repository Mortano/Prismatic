#pragma once

#include "Memory\peAllocator.h"
#include <memory>

namespace pe {

namespace detail {
struct PE_UTIL_API peAllocatableDeleter {
  template <typename T> void operator()(T *ptr) const { Delete(ptr); }
};
} // namespace detail

//! \brief unique_ptr definition that supports the engine allocators
template <typename T, typename = std::enable_if_t<
                          std::is_base_of_v<peAllocatable, std::decay_t<T>>>>
using peUniquePtr = std::unique_ptr<T, detail::peAllocatableDeleter>;

//! \brief make_unique implementation for peUniquePtr
//! \param args Constructor arguments
//! \param allocator Allocator to use
//! \returns unique_ptr
template <typename T, typename... Args>
peUniquePtr<T> peMakeUnique(Args &&... args,
                            IAllocator *allocator = GlobalAllocator) {
  return peUniquePtr<T>{New<T>(std::forward<Args>(args)..., allocator)};
}

} // namespace pe
