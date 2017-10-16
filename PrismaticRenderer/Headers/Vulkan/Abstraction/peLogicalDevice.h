#pragma once

#include "DataStructures/peVector.h"
#include "Vulkan/Util/peVulkanObject.h"
#include <vulkan/vulkan.hpp>

namespace pe {
class pePhysicalDevice;
class peCommandPool;

/// <summary>
/// Abstraction for a logical device in vulkan
/// </summary>
class peLogicalDevice : public peVulkanObject<VkDevice> {
public:
  explicit peLogicalDevice(VkDevice device, uint32_t queueFamilyIndex,
                           uint32_t graphicsQueueIndex,
                           const pePhysicalDevice &physicalDevice);

   /// <summary>
   /// Tries to create a logical device from the first physical device with the given capabilities
   /// </summary>
   /// <param name="queueCapabilities">Capability flags</param>
   /// <param name="physicalDevices">All physical devices on the current machine</param>
   /// <returns>Logical device, or null if no matching physical device was found</returns>
  static std::unique_ptr<peLogicalDevice>
  CreateWithCapabilities(VkQueueFlagBits queueCapabilities,
                         const peVector<pePhysicalDevice> &physicalDevices);

  std::unique_ptr<peCommandPool> CreateCommandPool() const;

private:
  const pePhysicalDevice &_physicalDevice;
  const uint32_t _queueFamilyIndex;
  const uint32_t _graphicsQueueIndex;
  const VkQueue _graphicsQueue;
};

} // namespace pe
