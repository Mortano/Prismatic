#pragma once
#include "DataStructures/peVector.h"
#include "Vulkan/Util/peDeviceBoundVulkanObject.h"
#include "peCommandBuffer.h"

namespace pe {
class peLogicalDevice;

/// <summary>
/// Abstraction of vulkan command pool
/// </summary>
class peCommandPool : public peDeviceBoundVulkanObject<VkCommandPool> {
public:
  explicit peCommandPool(VkCommandPool pool, const peLogicalDevice &device);

  peCommandBuffer &AllocateCommandBuffer();
  void FreeAllBuffers();

private:
  peVector<peCommandBuffer> _activeCommandBuffers;
};

} // namespace pe
