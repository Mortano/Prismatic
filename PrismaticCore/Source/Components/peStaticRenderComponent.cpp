#include "Components\peStaticRenderComponent.h"

void pe::peStaticRenderComponentSystem::OnCreate(
    peStaticRenderComponent &component, const peEntity &entity) {
  PrismaticEngine.GetRenderer()->RegisterDrawableEntity(entity);
}

void pe::peStaticRenderComponentSystem::OnDestroy(
    peStaticRenderComponent &component, const peEntity &entity) {
  PrismaticEngine.GetRenderer()->DeregisterDrawableEntity(entity);
}
