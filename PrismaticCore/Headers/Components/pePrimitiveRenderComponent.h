#pragma once

#include "Entities\Entity.h"
#include "Rendering/pePrimitives.h"

#include "Rendering/peMaterial.h"
#include <variant>

#pragma warning(push)
#pragma warning(disable : 4251)

namespace pe {

struct pePrimitiveRenderComponent;

//! \brief Component system for primitive render components
struct PE_CORE_API pePrimitiveRenderComponentSystem
    : peComponentSystem<pePrimitiveRenderComponentSystem> {
  using Component_t = pePrimitiveRenderComponent;

  void OnCreate(Component_t &component, const peEntity &entity);
  void OnDestroy(Component_t &component, const peEntity &entity);
};

//! \brief Render component for analytical primitives
struct PE_CORE_API pePrimitiveRenderComponent
    : peComponent<pePrimitiveRenderComponent> {
  using System_t = pePrimitiveRenderComponentSystem;

  using Primitives_t = peSpherePrimitive;

  Primitives_t primitive;
  peMaterial::Handle_t material;
};

} // namespace pe

#pragma warning(pop)
