#pragma once

#include "DataStructures/peVector.h"
#include "Vulkan/Util/peVulkanObject.h"

namespace pe {

/// <summary>
/// Abstraction of a physical device
/// </summary>
class pePhysicalDevice {
public:
  explicit pePhysicalDevice(VkPhysicalDevice physicalDevice);

  auto GetHandle() const { return _handle; }
  auto GetFeatures() const { return _features; }
  auto GetProperties() const { return _properties; }
  const auto &GetQueueFamilyProperties() const {
    return _queueFamilyProperties;
  }

private:
  const VkPhysicalDevice _handle;
  VkPhysicalDeviceFeatures _features;
  VkPhysicalDeviceProperties _properties;
  peVector<VkQueueFamilyProperties> _queueFamilyProperties;
};

} // namespace pe
