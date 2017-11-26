#pragma once
#include "Subsystems/IRenderer.h"

#include "OpenGL\peGlRendererBackend.h"

namespace pe {

//! OpenGL implementation of the renderer
class peGlRenderer : public IRenderer {
public:
  peGlRenderer();
  ~peGlRenderer();

  void Init() override;
  void Shutdown() override;
  void Update(double delta) override;

  void RegisterDrawableEntity(const peEntity &entity) override;
  void DeregisterDrawableEntity(const peEntity &entity) override;

private:
  void RegisterRenderResource(const peWeakPtr<peRenderResource> &res) override;
  void
  DeregisterRenderResource(const peWeakPtr<peRenderResource> &res) override;

  peGlRendererBackend _backend;
};

} // namespace pe
