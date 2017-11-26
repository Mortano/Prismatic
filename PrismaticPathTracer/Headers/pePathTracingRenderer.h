#pragma once
#include "Components/peLightComponent.h"
#include "Components/pePrimitiveRenderComponent.h"
#include "Components/peStaticRenderComponent.h"
#include "Entities\Entity.h"
#include "Shapes/Triangle.h"
#include "Subsystems/IRenderer.h"
#include "Window/peGlWindow.h"

namespace pe {
struct peStaticRenderComponent;

class pePathTracingRenderer : public IRenderer {
public:
  void Init() override;
  void Shutdown() override;
  void Update(double deltaTime) override;

  void RegisterDrawableEntity(const peEntity &entity) override;
  void DeregisterDrawableEntity(const peEntity &entity) override;

private:
  void RegisterRenderResource(const peWeakPtr<peRenderResource> &res) override;
  void
  DeregisterRenderResource(const peWeakPtr<peRenderResource> &res) override;

  void GatherLights();
  void AddStaticMesh(const peStaticRenderComponent &comp,
                     const peEntity &entity);

  uint32_t _windowWidth, _windowHeight;
  std::unique_ptr<peGlWindow> _window;

  peVector<pePrimitiveRenderComponent::Handle_t> _primitiveEntites;
  peVector<peStaticRenderComponent::Handle_t> _staticEntities;
  peVector<pePointLightComponent::Handle_t> _pointLights;
  peVector<peDirectionalLightComponent::Handle_t> _directionalLights;
};

} // namespace pe
