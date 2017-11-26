#pragma once
#include "Components/pePrimitiveRenderComponent.h"
#include "Components/peStaticRenderComponent.h"
#include "DataStructures/peVector.h"
#include "Math\peCoordSys.h"
#include "Rendering/pePrimitives.h"
#include "Sampling/peLightSampler.h"
#include "Shapes/Intersectable.h"
#include "Shapes/Sphere.h"
#include "Shapes/Triangle.h"
#include <optional>
#include <span.h>

namespace pe {
struct Ray;
class BSDF;

//! \brief Encapsulates a ray hit
struct SceneHit {
  glm::vec3 hitPosition;
  glm::vec3 hitNormal;
  peCoordSys shadingCoordinateSystem;
  uint32_t primitiveID;
};

//! \brief Helper structure that stores information about primitives
template <typename T> struct PrimitiveHelper {
  PrimitiveHelper(const T &primitive, const BSDF &bsdf)
      : primitive(primitive), bsdf(bsdf) {}

  T primitive;
  const BSDF bsdf;
};

//! \brief Encapsulates all objects in the scene
class peScene {
public:
  bool Intersects(const Ray &ray) const;
  std::optional<SceneHit> GetIntersection(const Ray &ray) const;

  const BSDF &GetBSDFForPrimitive(uint32_t id) const;

  const auto &GetLights() const { return _lightSamplers; }

  void BuildScene(
      gsl::span<peComponentHandle<pePrimitiveRenderComponent>> primitives,
      gsl::span<peStaticRenderComponent::Handle_t> staticEntities,
      gsl::span<peComponentHandle<pePointLightComponent>> pointLights,
      gsl::span<peComponentHandle<peDirectionalLightComponent>>
          directionalLights);

private:
  void AddStaticMesh(const peStaticRenderComponent &comp,
                     const peEntity &entity);

  template <typename T> void AddIntersectable(const T &prim) {
    auto tmpPrim = prim;
    _intersectables.emplace_back(tmpPrim);

    // Map primitive index to bsdf index
    const auto primIdx = static_cast<uint32_t>(_intersectables.size() - 1);
    const auto bsdfIdx = static_cast<uint32_t>(_bsdfs.size() - 1);

    _bsdfIndices[primIdx] = bsdfIdx;
  }

  // TODO Here we will use an acceleration structure later on

  peVector<std::unique_ptr<peLightSampler>> _lightSamplers;

  peVector<Sphere> _spheres;
  peVector<std::unique_ptr<TriangleMesh>> _meshes;
  peVector<Triangle> _triangles;

  peVector<Intersectable> _intersectables;
  peUnorderedMap<uint32_t, uint32_t> _bsdfIndices;
  peVector<BSDF> _bsdfs;
};

} // namespace pe
