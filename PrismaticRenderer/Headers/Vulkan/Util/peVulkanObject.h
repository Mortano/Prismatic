#pragma once
#include <vulkan/vulkan.hpp>

namespace pe {

namespace {
template <typename T> void DestroyVulkanObject(T obj) {
  static_assert(std::_Always_false<T>::value,
                "Please specialize DestroyVulkanObject for your type!");
}

template <> void DestroyVulkanObject<VkInstance>(VkInstance obj) {
  vkDestroyInstance(obj, nullptr);
}

template <> void DestroyVulkanObject<VkDevice>(VkDevice obj) {
  vkDestroyDevice(obj, nullptr);
}

} // namespace

/// <summary>
/// Abstraction that handles lifecycle of vulkan objects
/// </summary>
template <typename T> class peVulkanObject {
public:
  using Base_t = peVulkanObject<T>;

  explicit peVulkanObject(T handle) : _handle(handle) {}
  peVulkanObject(peVulkanObject<T> &&other) noexcept : _handle(other._handle) {
    other._handle = nullptr;
  }

  ~peVulkanObject() {
    if (_handle)
      DestroyVulkanObject(_handle);
  }

  peVulkanObject &operator=(peVulkanObject<T> &&other) noexcept {
    _handle = other._handle;
    other._handle = nullptr;
    return *this;
  }

  T operator()() const { return _handle; }

  T GetHandle() const { return _handle; }

private:
  T _handle;
};

} // namespace pe
