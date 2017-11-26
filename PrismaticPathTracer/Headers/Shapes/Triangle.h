#pragma once
#include "DataStructures/peVector.h"
#include "Math/AABB.h"
#include "Rendering/Utility/peBxDF.h"
#include <glm/detail/type_vec3.hpp>

namespace pe {
class Triangle;
struct Ray;

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
};

class TriangleMesh {
public:
  void SetGeometry(peVector<Vertex> vertices, peVector<uint32_t> indices);

  const auto &GetVertices() const { return _vertices; }
  const auto &GetIndices() const { return _indices; }

  void Refine(peVector<Triangle> &triangles) const;

private:
  peVector<Vertex> _vertices;
  peVector<uint32_t> _indices;
};

class Triangle {
public:
  Triangle(const TriangleMesh &mesh, uint32_t firstIndex);

  //! \brief First vertex
  const Vertex &V0() const;
  //! \brief Second vertex
  const Vertex &V1() const;
  //! \brief Third vertex
  const Vertex &V2() const;

  //! \brief Returns true if the given ray intersects this triangle
  bool Intersects(const Ray &ray, glm::vec3 *hitPos, glm::vec3 *hitNormal,
                  peCoordSys *shadingCoordinateSystem) const;

  //! \brief Returns the bounding box of this triangle
  AABB GetBounds() const;

private:
  const TriangleMesh &_mesh;
  uint32_t _firstIndex;
};

} // namespace pe
