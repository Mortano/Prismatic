#include "Rendering\peMesh.h"
#include "Exceptions/peExceptions.h"
#include "Syntax\peFormatter.h"
#include <numeric>

namespace pe {
uint32_t VertexAttributeElementCount(VertexAttribute attribute) {
  switch (attribute) {
  case pe::VertexAttribute::Position:
    return 3;
  case pe::VertexAttribute::Normal:
    return 3;
  case pe::VertexAttribute::TextureCoordinate:
    return 2;
  case pe::VertexAttribute::GenericField1Element:
    return 1;
  case pe::VertexAttribute::GenericField2Elements:
    return 2;
  case pe::VertexAttribute::GenericField3Elements:
    return 3;
  case pe::VertexAttribute::GenericField4Elements:
    return 4;
  default:
    throw std::runtime_error(Format()
                             << "Unrecognized VertexAttribute literal: "
                             << static_cast<uint32_t>(attribute));
  }
}

uint32_t VertexDataTypeSize(VertexDataType dataType) {
  switch (dataType) {
  case pe::VertexDataType::Byte:
    return 1;
  case pe::VertexDataType::UByte:
    return 1;
  case pe::VertexDataType::Int:
    return 4;
  case pe::VertexDataType::UInt:
    return 4;
  case pe::VertexDataType::Float:
    return 4;
  default:
    throw std::runtime_error(Format() << "Unrecognized VertexDataType literal: "
                                      << static_cast<uint32_t>(dataType));
  }
}

VertexComponent::VertexComponent(VertexAttribute attribute,
                                 VertexDataType data_type)
    : attribute(attribute), dataType(data_type) {}

uint32_t VertexLayout::SizeOfVertex() const {
  return std::accumulate(
      components.begin(), components.end(), 0u,
      [](auto accum, const auto &channel) {
        return accum + VertexAttributeElementCount(channel.attribute) *
                           VertexDataTypeSize(channel.dataType);
      });
}

peMesh::peMesh(VertexLayout vertexLayout) : _vertexLayout(vertexLayout) {}

void peMesh::SetVertexData(const peVector<char> &vertexData) {
#ifdef _DEBUG
  auto vertexSize = _vertexLayout.SizeOfVertex();
  if ((vertexData.size() % vertexSize) != 0)
    throw std::runtime_error{
        "Vertex data size must be a multiple of the size of a single vertex!"};
#endif
  auto mutableStorage = GetMutableDataStorage();
  mutableStorage->_vertexData = vertexData;
}

void peMesh::SetIndexData(const peVector<uint32_t> &indexData) {
  auto mutableStorage = GetMutableDataStorage();
  mutableStorage->_indexData = indexData;
}

const peMeshDataStorage &peMesh::GetData() const { return GetDataStorage(); }
} // namespace pe
