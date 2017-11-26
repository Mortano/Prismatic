#include "Components\pePrimitiveRenderComponent.h"
#include "peEngine.h"

void pe::pePrimitiveRenderComponentSystem::OnCreate(Component_t &component,
                                                    const peEntity &entity) {
  PrismaticEngine.GetRenderer()->RegisterDrawableEntity(entity);
}

void pe::pePrimitiveRenderComponentSystem::OnDestroy(Component_t &component,
                                                     const peEntity &entity) {
  PrismaticEngine.GetRenderer()->DeregisterDrawableEntity(entity);
}
