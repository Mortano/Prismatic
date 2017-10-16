#pragma once

#include <vulkan/vulkan.hpp>
#include "Vulkan/Util/peVulkanObject.h"
#include "DataStructures/peVector.h"
#include "pePhysicalDevice.h"

namespace pe
{
   
   class peVulkanInstance : public peVulkanObject<VkInstance>
   {
   public:
      static peVulkanInstance& GetInstance();
      
      const auto& GetPhysicalDevices() const { return _physicalDevices; }

   private:
      peVulkanInstance();
      ~peVulkanInstance() = default;

      peVector<pePhysicalDevice> _physicalDevices;
   };

}
