#pragma once
#include "peEngine.h"
#include <vulkan/vulkan.hpp>

namespace pe {

namespace detail {

constexpr const char *ResultCodeToString(const VkResult res) {
  switch (res) {
#define CASE(r)                                                                \
  case r:                                                                      \
    return #r;
    CASE(VK_SUCCESS)
    CASE(VK_NOT_READY)
    CASE(VK_TIMEOUT)
    CASE(VK_EVENT_SET)
    CASE(VK_EVENT_RESET)
    CASE(VK_INCOMPLETE)
    CASE(VK_ERROR_OUT_OF_HOST_MEMORY)
    CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY)
    CASE(VK_ERROR_INITIALIZATION_FAILED)
    CASE(VK_ERROR_DEVICE_LOST)
    CASE(VK_ERROR_MEMORY_MAP_FAILED)
    CASE(VK_ERROR_LAYER_NOT_PRESENT)
    CASE(VK_ERROR_EXTENSION_NOT_PRESENT)
    CASE(VK_ERROR_FEATURE_NOT_PRESENT)
    CASE(VK_ERROR_INCOMPATIBLE_DRIVER)
    CASE(VK_ERROR_TOO_MANY_OBJECTS)
    CASE(VK_ERROR_FORMAT_NOT_SUPPORTED)
    CASE(VK_ERROR_FRAGMENTED_POOL)
    CASE(VK_ERROR_SURFACE_LOST_KHR)
    CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
    CASE(VK_SUBOPTIMAL_KHR)
    CASE(VK_ERROR_OUT_OF_DATE_KHR)
    CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
    CASE(VK_ERROR_VALIDATION_FAILED_EXT)
    CASE(VK_ERROR_INVALID_SHADER_NV)
    CASE(VK_ERROR_OUT_OF_POOL_MEMORY_KHR)
    CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR)
#undef CASE
  default:
    return "Unrecognized error code";
  }
}

inline void VulkanVerify(const VkResult res, const VkResult expected,
                         const char *msg) {
  if (res == expected)
    return;
  PrismaticEngine.GetLogging()->LogError(
      "Operation %s failed! Expected [%s], got [%s]", msg,
      ResultCodeToString(expected), ResultCodeToString(res));
}
} // namespace detail

} // namespace pe

#define _STR(x) #x
#define STR(x) _STR(x)

#define VK_EXPECT(expected, call) pe::detail::VulkanVerify( (call), expected, STR(call) )
#define VK_EXPECT_SUCCESS(call) pe::detail::VulkanVerify( (call), VK_SUCCESS, STR(call) )
