#pragma once
#include "Vulkan\Abstraction\peLogicalDevice.h"

namespace pe {

namespace {

template <typename T>
void DestroyDeviceBoundVulkanObject(T, const pe::peLogicalDevice &) {
  static_assert(
      std::_Always_false<T>::value,
      "Please specialize DestroyDeviceBoundVulkanObject for your type!");
}

template <>
void DestroyDeviceBoundVulkanObject<VkCommandPool>(
    VkCommandPool obj, const pe::peLogicalDevice &device) {
  vkDestroyCommandPool(device.GetHandle(), obj, nullptr);
}

template <>
void DestroyDeviceBoundVulkanObject<VkCommandBuffer>(
    VkCommandBuffer obj, const pe::peLogicalDevice &device) {
  // Does nothing because we need the command pool to free the command buffer.
  // This will be done in the implementation  of peCommandBuffer
}
} // namespace

/// <summary>
/// Abstraction that handles lifecycle of vulkan objects that are bound to a
/// specific logical device
/// </summary>
template <typename T> class peDeviceBoundVulkanObject {
public:
  using Base_t = peDeviceBoundVulkanObject<T>;

  peDeviceBoundVulkanObject(T handle, const peLogicalDevice &device)
      : _handle(handle), _device(device) {}

  ~peDeviceBoundVulkanObject() {
    if (_handle)
      DestroyDeviceBoundVulkanObject(_handle, _device);
  }

  T GetHandle() const { return _handle; }

  const peLogicalDevice &GetDevice() const { return _device; }

private:
  T _handle;
  const peLogicalDevice &_device;
};

} // namespace pe
