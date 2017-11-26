#pragma once
#include "DataStructures/peVector.h"
#include "peRenderResource.h"
#include <memory>

#pragma warning(push)
#pragma warning(disable : 4251)

namespace pe {

//! \brief Different vertex attributes for a vertex in a polygonal mesh
enum class PE_CORE_API VertexAttribute {
  Position,
  Normal,
  TextureCoordinate,
  GenericField1Element,
  GenericField2Elements,
  GenericField3Elements,
  GenericField4Elements
};

//! \brief Different data types for a vertex attribute in a polygonal mesh
enum class PE_CORE_API VertexDataType { Byte, UByte, Int, UInt, Float };

//! \brief Returns the number of elements in the given vertex attribute
//! \param attribute Vertex attribute
//! \returns Number of elements in the vertex attribute
uint32_t PE_CORE_API VertexAttributeElementCount(VertexAttribute attribute);

//! \brief Returns the size in bytes of the given VertexDataType
//! \param dataType VertexDataType
//! \returns Size in byte of the VertexDataType
uint32_t PE_CORE_API VertexDataTypeSize(VertexDataType dataType);

//! \brief Describes a single component of a vertex in a polygonal mesh. It
//! stores both the type/usage of the component as well as the data type used to
//! represent it
struct PE_CORE_API VertexComponent {
  VertexComponent(VertexAttribute attribute, VertexDataType dataType);

  VertexAttribute attribute;
  VertexDataType dataType;
};

//! \brief Describes the layout of a single vertex in the mesh. Each vertex is
//! made up of the components
struct PE_CORE_API VertexLayout {
  peVector<VertexComponent> components;

  //! \brief Returns the size in bytes of a single vertex due to this layout
  //! \returns Vertex size in bytes
  uint32_t SizeOfVertex() const;
};

//! \brief Data storage for polygonal meshes
struct PE_CORE_API peMeshDataStorage {
  peVector<char> _vertexData;
  peVector<uint32_t> _indexData;
};

//! \brief Polygonal mesh
struct PE_CORE_API peMesh : peRenderResourceBase<peMesh, peMeshDataStorage> {
  explicit peMesh(VertexLayout vertexLayout);

  void SetVertexData(const peVector<char> &vertexData);
  void SetIndexData(const peVector<uint32_t> &indexData);

  const peMeshDataStorage &GetData() const;
  const auto &GetVertexLayout() const { return _vertexLayout; }

private:
  const VertexLayout _vertexLayout;
};

} // namespace pe

#pragma warning(pop)
