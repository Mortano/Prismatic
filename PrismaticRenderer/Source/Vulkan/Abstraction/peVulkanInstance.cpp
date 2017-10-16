#include "Vulkan/Abstraction/peVulkanInstance.h"
#include "Vulkan/Util/peCreateInfoHelper.h"
#include "Vulkan/Util/peVulkanVerify.h"

/// <summary>
/// Creates a Vulkan instance
/// </summary>
/// <returns>Vulkan instance</returns>
static VkInstance CreateVkInstance() {
  auto appInfo = pe::ApplicationInfo();
  appInfo.apiVersion = VK_API_VERSION_1_0;
  appInfo.applicationVersion = 1;
  appInfo.engineVersion = 1;
  appInfo.pApplicationName = "PrismaticEngine";
  appInfo.pEngineName = "PrismaticEngine";

  auto createInfo = pe::InstanceCreateInfo();

  createInfo.pApplicationInfo = &appInfo;

  std::vector<const char *> layerNames = {
      "VK_LAYER_LUNARG_standard_validation"};
  // TODO Check that requested layers actually are supported
  createInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());
  createInfo.ppEnabledLayerNames = layerNames.data();

  std::vector<const char *> extensionNames = {
      VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(extensionNames.size());
  createInfo.ppEnabledExtensionNames = extensionNames.data();

  VkInstance ret;
  VK_EXPECT_SUCCESS(vkCreateInstance(&createInfo, nullptr, &ret));
  return ret;
}

static pe::peVector<pe::pePhysicalDevice>
EnumeratePhysicalDevices(VkInstance instance) {
  pe::peVector<VkPhysicalDevice> physicalDevices;
  uint32_t numPhysicalDevices;
  VK_EXPECT_SUCCESS(
      vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, nullptr));

  if (!numPhysicalDevices)
    return {};

  physicalDevices.resize(numPhysicalDevices);
  VK_EXPECT_SUCCESS(vkEnumeratePhysicalDevices(instance, &numPhysicalDevices,
                                               physicalDevices.data()));

  return Transform(physicalDevices, [](const auto device) {
    return pe::pePhysicalDevice{device};
  });
}

pe::peVulkanInstance &pe::peVulkanInstance::GetInstance() {
  static peVulkanInstance s_instance;
  return s_instance;
}

pe::peVulkanInstance::peVulkanInstance() : Base_t(CreateVkInstance()) {
  _physicalDevices = EnumeratePhysicalDevices(GetHandle());
  if (!_physicalDevices.size())
    throw std::runtime_error{"No physical devices found!"};
}
