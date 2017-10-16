#include "Vulkan/Abstraction/pePhysicalDevice.h"

pe::pePhysicalDevice::pePhysicalDevice(VkPhysicalDevice physicalDevice)
    : _handle(physicalDevice) {
  vkGetPhysicalDeviceProperties(GetHandle(), &_properties);
  vkGetPhysicalDeviceFeatures(GetHandle(), &_features);

  uint32_t queueFamiliesCount;
  vkGetPhysicalDeviceQueueFamilyProperties(GetHandle(), &queueFamiliesCount, nullptr);
  _queueFamilyProperties.resize(queueFamiliesCount);
  vkGetPhysicalDeviceQueueFamilyProperties(GetHandle(), &queueFamiliesCount, _queueFamilyProperties.data());
}
