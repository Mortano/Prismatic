#include "vulkan/Abstraction/peCommandBuffer.h"
#include "vulkan/Abstraction/peCommandPool.h"

pe::peCommandBuffer::peCommandBuffer(VkCommandBuffer buffer,
                                     const peCommandPool &pool,
                                     const peLogicalDevice &device)
    : Base_t(buffer, device), _pool(pool) {}

pe::peCommandBuffer::~peCommandBuffer() {
  auto handle = GetHandle();
  if (handle)
    vkFreeCommandBuffers(GetDevice().GetHandle(), _pool.GetHandle(), 1,
                         &handle);
}
