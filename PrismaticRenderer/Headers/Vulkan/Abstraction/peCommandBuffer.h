#pragma once
#include "vulkan/Util/peDeviceBoundVulkanObject.h"

namespace pe {

class peCommandPool;

class peCommandBuffer : public peDeviceBoundVulkanObject<VkCommandBuffer> {
public:
  explicit peCommandBuffer(VkCommandBuffer buffer, const peCommandPool &pool,
                           const peLogicalDevice &device);

  ~peCommandBuffer();

private:
  const peCommandPool &_pool;
};

} // namespace pe
