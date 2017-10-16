#pragma once

#include <vulkan/vulkan.hpp>

namespace pe
{
   
   VkInstanceCreateInfo InstanceCreateInfo();

   VkApplicationInfo ApplicationInfo();

   VkDeviceCreateInfo DeviceCreateInfo();

   VkDeviceQueueCreateInfo DeviceQueueCreateInfo();

   VkCommandPoolCreateInfo CommandPoolCreateInfo();

   VkCommandBufferAllocateInfo CommandBufferAllocateInfo();

}