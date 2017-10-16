#include "Vulkan/peVulkanRenderer.h"
#include "Vulkan/Abstraction/peVulkanInstance.h"

#include <algorithm>
#include <optional>
#include "Algorithms/peAlgorithms.h"
#include "Vulkan/Util/peCreateInfoHelper.h"
#include "Vulkan/Util/peVulkanVerify.h"

static std::unique_ptr<pe::peLogicalDevice> CreateLogicalDevice() {
  auto &vkInstance = pe::peVulkanInstance::GetInstance();
  auto &physicalDevices = vkInstance.GetPhysicalDevices();
  return pe::peLogicalDevice::CreateWithCapabilities(VK_QUEUE_GRAPHICS_BIT, physicalDevices);
}

void pe::peVulkanRenderer::Init()
{
   _logicalDevice = CreateLogicalDevice();
   if (!_logicalDevice) throw std::runtime_error{ "Could not create a logical device because no physical device with a graphics-capable queue was found!" };
}

void pe::peVulkanRenderer::Shutdown() {}

void pe::peVulkanRenderer::Update(double deltaTime) {
  // Extract draw calls from scene graph
  // Process draw calls
}
