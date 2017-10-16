#include "Vulkan/Abstraction/peCommandPool.h"
#include "Vulkan/Abstraction/peLogicalDevice.h"
#include "Vulkan/Util/peVulkanVerify.h"

pe::peCommandPool::peCommandPool(VkCommandPool pool,
                                 const peLogicalDevice &logicalDevice)
    : Base_t(pool, logicalDevice) {}

pe::peCommandBuffer &pe::peCommandPool::AllocateCommandBuffer() {
  auto allocateInfo = vk::CommandBufferAllocateInfo();
  allocateInfo.commandBufferCount = 1;
  allocateInfo.commandPool = vk::CommandPool{GetHandle()};
  allocateInfo.level =
      static_cast<vk::CommandBufferLevel>(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  VkCommandBuffer buffer;
  VK_EXPECT_SUCCESS(vkAllocateCommandBuffers(
      GetDevice().GetHandle(),
      &static_cast<const VkCommandBufferAllocateInfo &>(allocateInfo),
      &buffer));

  _activeCommandBuffers.emplace_back(buffer, *this, GetDevice());
  return _activeCommandBuffers.back();
}

void pe::peCommandPool::FreeAllBuffers() { _activeCommandBuffers.clear(); }
