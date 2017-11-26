#include "Scene\peScene.h"
#include "Components/peTransformComponent.h"
#include "Math/peCoordSys.h"
#include "Rendering/peMesh.h"
#include "Shapes/Triangle.h"
#include "Util/Intersections.h"

bool pe::peScene::Intersects(const Ray &ray) const {
  auto wasHit = false;
  for (auto &intersectable : _intersectables) {
    wasHit |= intersectable.Intersects(ray, nullptr, nullptr, nullptr);
  }
  return wasHit;
}

std::optional<pe::SceneHit> pe::peScene::GetIntersection(const Ray &ray) const {
  glm::vec3 hitPos, hitNormal;
  peCoordSys shadingCoordinateSystem;
  std::optional<uint32_t> hitIdx;
  for (size_t idx = 0; idx < _intersectables.size(); ++idx) {
    auto &intersectable = _intersectables[idx];
    const auto wasHit = intersectable.Intersects(ray, &hitPos, &hitNormal,
                                                 &shadingCoordinateSystem);
    if (wasHit)
      hitIdx = static_cast<uint32_t>(idx);
  }
  if (!hitIdx)
    return {};
  return std::make_optional(
      SceneHit{hitPos, hitNormal, shadingCoordinateSystem, hitIdx.value()});
}

const pe::BSDF &pe::peScene::GetBSDFForPrimitive(uint32_t id) const {
  if (id >= _intersectables.size())
    throw std::runtime_error("Primitive ID does not exist!");
  const auto bsdfIdx = _bsdfIndices.at(id);
  return _bsdfs[bsdfIdx];
}

void pe::peScene::BuildScene(
    gsl::span<peComponentHandle<pePrimitiveRenderComponent>> primitives,
    gsl::span<peStaticRenderComponent::Handle_t> staticEntities,
    gsl::span<peComponentHandle<pePointLightComponent>> pointLights,
    gsl::span<peComponentHandle<peDirectionalLightComponent>>
        directionalLights) {
  for (auto &primComponent : primitives) {
    auto &sphere = primComponent->primitive;
    auto &material = primComponent->material;
    const auto offlineData = material->GetOfflineData();
    if (!offlineData)
      continue;

    // TODO Transform sphere to world space

    _spheres.emplace_back(sphere);
    _bsdfs.emplace_back(*offlineData);

    AddIntersectable(_spheres.back());
  }

  for (auto staticEntity : staticEntities) {
    AddStaticMesh(*staticEntity, staticEntity.GetEntity());
  }

  for (auto &light : pointLights) {
    _lightSamplers.push_back(std::make_unique<pePointLightSampler>(*light));
  }

  for (auto &light : directionalLights) {
    // TODO
    //_lightSamplers.push_back(std::make_unique<peDirect>(*light));
  }
}

void pe::peScene::AddStaticMesh(const peStaticRenderComponent &comp,
                                const peEntity &entity) {
  auto &material = comp.material;
  if (!material.IsAlive())
    return;
  const auto offlineData = material->GetOfflineData();
  if (!offlineData) {
    PrismaticEngine.GetLogging()->LogError(
        "Mesh has no offline material data!");
    return;
  }

  auto &mesh = *comp.mesh;
  auto &meshData = mesh.GetData();
  auto &vertexLayout = mesh.GetVertexLayout();

  // TODO Support different vertex layouts
  if (vertexLayout.components.size() != 2 ||
      vertexLayout.components[0].attribute != VertexAttribute::Position ||
      vertexLayout.components[0].dataType != VertexDataType::Float ||
      vertexLayout.components[1].attribute != VertexAttribute::Normal ||
      vertexLayout.components[1].dataType != VertexDataType::Float) {
    PrismaticEngine.GetLogging()->LogError("Invalid vertex layout on mesh!");
    return;
  }

  peVector<Vertex> vertices{
      reinterpret_cast<Vertex const *>(meshData._vertexData.data()),
      reinterpret_cast<Vertex const *>(meshData._vertexData.data() +
                                       meshData._vertexData.size())};

  auto transformComponent = entity.GetComponent<peTransformComponent>();
  if (transformComponent) {
    // Get object-to-world matrix
    auto &objToWorld = transformComponent->transformation;
    // Transform geometry to world space
    for (auto &vertex : vertices) {
      auto worldPos4d = (objToWorld * glm::vec4{vertex.position, 1.f});
      vertex.position = {worldPos4d.x, worldPos4d.y, worldPos4d.z};

      auto normal4d = (objToWorld * glm::vec4(vertex.normal, 0.f));
      vertex.normal =
          glm::normalize(glm::vec3{normal4d.x, normal4d.y, normal4d.z});
    }
  }

  auto newMesh = std::make_unique<TriangleMesh>();
  newMesh->SetGeometry(std::move(vertices), meshData._indexData);

  const auto oldTriangleCount = _triangles.size();
  newMesh->Refine(_triangles);

  _meshes.push_back(std::move(newMesh));

  _bsdfs.emplace_back(*offlineData);

  for (auto idx = oldTriangleCount; idx < _triangles.size(); ++idx) {
    AddIntersectable(_triangles[idx]);
  }
}
