#pragma once
#include "Subsystems/IRenderer.h"
#include "peVulkanRendererBackend.h"
#include <memory>
#include "Abstraction/peLogicalDevice.h"

namespace pe
{
   class pePhysicalDevice;

   class peVulkanRenderer : public IRenderer
   {
   public:
      void Init() override;
      void Shutdown() override;
      void Update(double deltaTime) override;
   private:
      peVulkanRendererBackend _backend;      

      pePhysicalDevice* _physicalDevice;
      std::unique_ptr<peLogicalDevice> _logicalDevice;
   };

}
