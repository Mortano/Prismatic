#include "Vulkan/Util/peCreateInfoHelper.h"

VkInstanceCreateInfo pe::InstanceCreateInfo()
{
   VkInstanceCreateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   info.pNext = nullptr;
   return info;
}

VkApplicationInfo pe::ApplicationInfo()
{
   VkApplicationInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   info.pNext = nullptr;
   return info;
}

VkDeviceCreateInfo pe::DeviceCreateInfo()
{
   VkDeviceCreateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   info.pNext = nullptr;
   return info;
}

VkDeviceQueueCreateInfo pe::DeviceQueueCreateInfo()
{
   VkDeviceQueueCreateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
   info.pNext = nullptr;
   return info;
}

VkCommandPoolCreateInfo pe::CommandPoolCreateInfo()
{
   VkCommandPoolCreateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   info.pNext = nullptr;
   return info;
}

VkCommandBufferAllocateInfo pe::CommandBufferAllocateInfo()
{
   VkCommandBufferAllocateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   info.pNext = nullptr;
   return info;
}
