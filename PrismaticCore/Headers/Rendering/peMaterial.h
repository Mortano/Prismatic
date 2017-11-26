#pragma once
#include "Rendering\Utility\peBxDF.h"
#include "Type/peColor.h"
#include "peRenderResource.h"

#include <variant>

#pragma warning(push)
#pragma warning(disable : 4251)

namespace pe {

//! \brief Material properties for real-time rendering
struct PE_CORE_API peRealTimeMaterialData {
  RGBA_8Bit albedo;
  float roughness;
  float metallic;
};

//! \brief Material properties for offline rendering (physically correct and
//! whatnot)
using peOfflineMaterialData = BSDF;

struct PE_CORE_API peMaterialDataStorage {
  std::variant<peRealTimeMaterialData, peOfflineMaterialData> data;
};

//! \brief Material that defines object surface properties
class PE_CORE_API peMaterial
    : public peRenderResourceBase<peMaterial, peMaterialDataStorage> {
public:
  void SetRealTimeData(const peRealTimeMaterialData &data);
  void SetOfflineData(const BSDF &data);

  peRealTimeMaterialData const *GetRealTimeData() const;
  peOfflineMaterialData const *GetOfflineData() const;
};

} // namespace pe

#pragma warning(pop)
