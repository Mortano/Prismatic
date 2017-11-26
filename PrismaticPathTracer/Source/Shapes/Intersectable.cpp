#include "Shapes\Intersectable.h"

pe::Intersectable::Intersectable(const Intersectable &other) {
  other.GetHolder().Clone(std::addressof(_storage));
}

pe::Intersectable::~Intersectable() {
  // TODO Assumption here is that Intersectable always has a valid value, but
  // maybe we don't want that
  reinterpret_cast<HolderBase *>(std::addressof(_storage))->~HolderBase();
  _deleted = true;
}

bool pe::Intersectable::Intersects(const Ray &ray, glm::vec3 *hitPos,
                                   glm::vec3 *hitNormal,
                                   peCoordSys *shadingCoordinateSystem) const {
  return GetHolder().Intersects(ray, hitPos, hitNormal,
                                shadingCoordinateSystem);
}

pe::AABB pe::Intersectable::GetBounds() const {
  return GetHolder().GetBounds();
}

pe::Intersectable::HolderBase &pe::Intersectable::GetHolder() const {
  return *reinterpret_cast<HolderBase *>(std::addressof(_storage));
}
