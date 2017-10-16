#include "Vulkan/Abstraction/peLogicalDevice.h"
#include "Algorithms/peAlgorithms.h"
#include "Vulkan/Abstraction/pePhysicalDevice.h"
#include "Vulkan/Util/peCreateInfoHelper.h"
#include "Vulkan/Util/peVulkanVerify.h"
#include "vulkan/Abstraction/peCommandPool.h"
#include <optional>

static VkQueue GetQueue(VkDevice device, uint32_t queueFamilyIndex,
                        uint32_t queueIndex) {
  VkQueue ret;
  vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &ret);
  return ret;
}

pe::peLogicalDevice::peLogicalDevice(VkDevice device, uint32_t queueFamilyIndex,
                                     uint32_t graphicsQueueIndex,
                                     const pePhysicalDevice &physicalDevice)
    : Base_t(device), _physicalDevice(physicalDevice),
      _queueFamilyIndex(queueFamilyIndex),
      _graphicsQueueIndex(graphicsQueueIndex),
      _graphicsQueue(GetQueue(device, queueFamilyIndex, graphicsQueueIndex)) {}

std::unique_ptr<pe::peLogicalDevice>
pe::peLogicalDevice::CreateWithCapabilities(
    VkQueueFlagBits queueCapabilities,
    const peVector<pePhysicalDevice> &physicalDevices) {
  // Lambda that finds (and, if possible, returns) a physical device and queue
  // with certain capabilities
  auto findQueue = [=](const pe::pePhysicalDevice &device)
      -> std::optional<std::tuple<uint32_t, VkQueueFamilyProperties,
                                  const pe::pePhysicalDevice &>> {
    auto &queueFamilyProperties = device.GetQueueFamilyProperties();
    auto firstGraphicsQueueIter = std::find_if(
        queueFamilyProperties.begin(), queueFamilyProperties.end(),
        [=](const auto &queueProperty) {
          return (queueProperty.queueFlags & queueCapabilities) != 0;
        });

    if (firstGraphicsQueueIter == queueFamilyProperties.end())
      return std::nullopt;
    auto queueFamilyIndex = static_cast<uint32_t>(
        std::distance(queueFamilyProperties.begin(), firstGraphicsQueueIter));

    // TODO Check for surface support on this device! We need a WinKhrSurface
    // for this
    // vkGetPhysicalDeviceSurfaceSupportKHR(device.GetHandle(),
    // queueFamilyIndex, )

    return {{queueFamilyIndex, *firstGraphicsQueueIter, device}};
  };

  auto graphicsQueue = pe::FindFirst(
      physicalDevices, [&](const auto &device) { return findQueue(device); });

  if (!graphicsQueue) {
    return nullptr;
  }

  auto deviceCreateInfo = pe::DeviceCreateInfo();

  auto queueCreateInfo = pe::DeviceQueueCreateInfo();
  queueCreateInfo.flags = 0;
  queueCreateInfo.queueCount = 1;
  queueCreateInfo.queueFamilyIndex = std::get<0>(*graphicsQueue);
  float queuePriorities[] = {1.0};
  queueCreateInfo.pQueuePriorities = queuePriorities;

  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
  deviceCreateInfo.queueCreateInfoCount = 1;

  std::vector<const char *> enabledLayers = {
      "VK_LAYER_LUNARG_standard_validation"};
  deviceCreateInfo.enabledLayerCount =
      static_cast<uint32_t>(enabledLayers.size());
  deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();

  std::vector<const char *> enabledExtensions = {
      "VK_KHR_surface", VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(enabledExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

  auto &physicalDeviceForQueue = std::get<2>(*graphicsQueue);
  VkDevice device;
  VK_EXPECT_SUCCESS(vkCreateDevice(physicalDeviceForQueue.GetHandle(),
                                   &deviceCreateInfo, nullptr, &device));

  // Graphics queue index is always 0 for the moment
  return std::make_unique<peLogicalDevice>(
      device, queueCreateInfo.queueFamilyIndex, 0, physicalDeviceForQueue);
}

std::unique_ptr<pe::peCommandPool>
pe::peLogicalDevice::CreateCommandPool() const {
  auto createInfo = CommandPoolCreateInfo();
  createInfo.flags = 0;
  createInfo.queueFamilyIndex = _queueFamilyIndex;
  VkCommandPool pool;
  VK_EXPECT_SUCCESS(
      vkCreateCommandPool(GetHandle(), &createInfo, nullptr, &pool));
  return std::make_unique<pe::peCommandPool>(pool, *this);
}
