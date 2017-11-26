
#include "OpenGL\peGlRenderer.h"

namespace pe {

peGlRenderer::peGlRenderer() {}

peGlRenderer::~peGlRenderer() {}

void peGlRenderer::Init() { _backend.Init(); }

void peGlRenderer::Shutdown() { _backend.Shutdown(); }

void peGlRenderer::Update(double delta) {}

void peGlRenderer::RegisterDrawableEntity(const peEntity &entity) {}

void peGlRenderer::DeregisterDrawableEntity(const peEntity &entity) {}

void peGlRenderer::RegisterRenderResource(
    const peWeakPtr<peRenderResource> &res) {}

void peGlRenderer::DeregisterRenderResource(
    const peWeakPtr<peRenderResource> &res) {}
} // namespace pe
