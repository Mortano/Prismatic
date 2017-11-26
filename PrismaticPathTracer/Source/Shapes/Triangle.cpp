#include "Shapes\Triangle.h"
#include "Util\Intersections.h"
#include <iostream>

void pe::TriangleMesh::SetGeometry(peVector<Vertex> vertices,
                                   peVector<uint32_t> indices) {
  if ((indices.size() % 3) != 0)
    throw std::runtime_error{"Incorrect indices size!"};
  _vertices = std::move(vertices);
  _indices = std::move(indices);
}

void pe::TriangleMesh::Refine(peVector<Triangle> &triangles) const {
  triangles.reserve(triangles.size() + _indices.size() / 3);
  for (size_t idx = 0; idx < _indices.size(); idx += 3) {
    auto &tri = triangles.emplace_back(*this, static_cast<uint32_t>(idx));
    std::cout << "Triangle:\n";
    std::cout << "\t(" << tri.V0().normal.x << ";" << tri.V0().normal.y << ";"
              << tri.V0().normal.z << ")" << std::endl;
    std::cout << "\t(" << tri.V1().normal.x << ";" << tri.V1().normal.y << ";"
              << tri.V1().normal.z << ")" << std::endl;
    std::cout << "\t(" << tri.V2().normal.x << ";" << tri.V2().normal.y << ";"
              << tri.V2().normal.z << ")" << std::endl;
  }
}

pe::Triangle::Triangle(const TriangleMesh &mesh, uint32_t firstIndex)
    : _mesh(mesh), _firstIndex(firstIndex) {}

const pe::Vertex &pe::Triangle::V0() const {
  auto &vs = _mesh.GetVertices();
  auto &is = _mesh.GetIndices();
  return vs[is[_firstIndex]];
}

const pe::Vertex &pe::Triangle::V1() const {
  auto &vs = _mesh.GetVertices();
  auto &is = _mesh.GetIndices();
  return vs[is[_firstIndex + 1]];
}

const pe::Vertex &pe::Triangle::V2() const {
  auto &vs = _mesh.GetVertices();
  auto &is = _mesh.GetIndices();
  return vs[is[_firstIndex + 2]];
}

bool pe::Triangle::Intersects(const Ray &ray, glm::vec3 *hitPos,
                              glm::vec3 *hitNormal,
                              peCoordSys *shadingCoordinateSystem) const {
  return RayTriangleIntersection(ray, V0(), V1(), V2(), hitPos, hitNormal,
                                 shadingCoordinateSystem);
}

pe::AABB pe::Triangle::GetBounds() const {
  return Union(AABB{V0().position, V1().position}, V2().position);
}
