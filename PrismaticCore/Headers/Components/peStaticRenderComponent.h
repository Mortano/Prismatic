#pragma once
#include "Entities\Entity.h"
#include "Rendering/peMaterial.h"
#include "Rendering\peMesh.h"

#pragma warning(push)
#pragma warning(disable : 4251)

namespace pe {

struct peStaticRenderComponent;

//! \brief System for operations regarding peStaticRenderComponent
struct PE_CORE_API peStaticRenderComponentSystem
    : peComponentSystem<peStaticRenderComponentSystem> {
  using Component_t = peStaticRenderComponent;

  void OnCreate(peStaticRenderComponent &component, const peEntity &entity);
  void OnDestroy(peStaticRenderComponent &component, const peEntity &entity);
};

//! \brief A component for rendering a static mesh (i.e. a mesh that doesn't
//! change or move)
struct PE_CORE_API peStaticRenderComponent
    : peComponent<peStaticRenderComponent> {
  using System_t = peStaticRenderComponentSystem;

  peMesh::Handle_t mesh;
  peMaterial::Handle_t material;
};

} // namespace pe

#pragma warning(pop)
