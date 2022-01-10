/**
 * @file Simulation/Appearances/ComplexAppearance.cpp
 * Implementation of class ComplexAppearance
 * @author Colin Graf
 */

#include "ComplexAppearance.h"
#include "Platform/Assert.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_map>

struct ComplexAppearanceDescriptor
{
  ComplexAppearanceDescriptor(const ComplexAppearance& appearance) :
    vertices(appearance.vertices),
    normals(appearance.normals),
    texCoords(appearance.texCoords),
    primitiveGroups(&appearance.primitiveGroups),
    normalsDefined(appearance.normalsDefined)
  {}

  ComplexAppearance::Vertices* vertices;
  ComplexAppearance::Normals* normals;
  ComplexAppearance::TexCoords* texCoords;
  const std::list<ComplexAppearance::PrimitiveGroup*>* primitiveGroups;
  bool normalsDefined;

  bool operator==(const ComplexAppearanceDescriptor& other) const
  {
    return vertices == other.vertices &&
           normalsDefined == other.normalsDefined &&
           (!normalsDefined || (normals == other.normals)) &&
           texCoords == other.texCoords &&
           std::equal(primitiveGroups->begin(), primitiveGroups->end(), other.primitiveGroups->begin(), other.primitiveGroups->end());
  }
};

struct ComplexAppearanceHasher
{
  std::size_t operator()(const ComplexAppearanceDescriptor& descriptor) const
  {
    // Don't hash primitive groups. The few collisions don't really hurt initialization performance.
    return ((std::hash<ComplexAppearance::Vertices*>()(descriptor.vertices) ^ (std::hash<ComplexAppearance::Normals*>()(descriptor.normalsDefined ? descriptor.normals : nullptr) << 1)) >> 1) ^ (std::hash<ComplexAppearance::TexCoords*>()(descriptor.texCoords) << 1);
  }
};

static std::unordered_map<ComplexAppearanceDescriptor, GraphicsContext::Mesh*, ComplexAppearanceHasher> meshCache;

void ComplexAppearance::PrimitiveGroup::addParent(Element& element)
{
  ComplexAppearance* complexAppearance = dynamic_cast<ComplexAppearance*>(&element);
  complexAppearance->primitiveGroups.push_back(this);
}

void ComplexAppearance::Vertices::addParent(Element& element)
{
  ComplexAppearance* complexAppearance = dynamic_cast<ComplexAppearance*>(&element);
  ASSERT(!complexAppearance->vertices);
  complexAppearance->vertices = this;
}

void ComplexAppearance::Normals::addParent(Element& element)
{
  ComplexAppearance* complexAppearance = dynamic_cast<ComplexAppearance*>(&element);
  ASSERT(!complexAppearance->normals);
  complexAppearance->normals = this;
  complexAppearance->normalsDefined = true;
}

void ComplexAppearance::TexCoords::addParent(Element& element)
{
  ComplexAppearance* complexAppearance = dynamic_cast<ComplexAppearance*>(&element);
  ASSERT(!complexAppearance->texCoords);
  complexAppearance->texCoords = this;
}

GraphicsContext::Mesh* ComplexAppearance::createMesh(GraphicsContext& graphicsContext)
{
  ASSERT(vertices);
  ASSERT(!primitiveGroups.empty());

  const ComplexAppearanceDescriptor descriptor(*this);
  if(const auto cachedMesh = meshCache.find(descriptor); cachedMesh != meshCache.end())
    return cachedMesh->second;

  const bool withTextureCoordinates = texCoords && surface->texture;
  const std::size_t verticesSize = vertices->vertices.size();
  if(!normalsDefined)
  {
    const Vertex* vertexLibrary = vertices->vertices.data();
    ASSERT(!normals);
    normals = new Normals;
    normals->normals.resize(verticesSize);
    Normal* vertexNormals = normals->normals.data();
    for(PrimitiveGroup* primitiveGroup : primitiveGroups)
    {
      ASSERT((primitiveGroup->vertices.size() % (primitiveGroup->mode == triangles ? 3 : 4)) == 0);
      for(auto iter = primitiveGroup->vertices.begin(), end = primitiveGroup->vertices.end(); iter != end;)
      {
        unsigned int i1 = *iter;
        if(i1 >= verticesSize)
          *iter = i1 = 0; // this is bullshit, but better than crashing because of incorrect input files
        ++iter;
        unsigned int i2 = *iter;
        if(i2 >= verticesSize)
          *iter = i2 = 0; // this is bullshit, but better than crashing because of incorrect input files
        ++iter;
        unsigned int i3 = *iter;
        if(i3 >= verticesSize)
          *iter = i3 = 0; // this is bullshit, but better than crashing because of incorrect input files
        ++iter;
        unsigned int i4 = 0;
        if(primitiveGroup->mode == quads)
        {
          i4 = *iter;
          if(i4 >= verticesSize)
            *iter = i4 = 0; // this is bullshit, but better than crashing because of incorrect input files
          ++iter;
        }

        const Vertex& p1 = vertexLibrary[i1];
        const Vertex& p2 = vertexLibrary[i2];
        const Vertex& p3 = vertexLibrary[i3];

        Vertex u(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
        Vertex v(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);
        Normal n(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x, 1);
        float len = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
        len = len == 0 ? 1.f : 1.f / len;
        n.x *= len;
        n.y *= len;
        n.z *= len;

        vertexNormals[i1] += n;
        vertexNormals[i2] += n;
        vertexNormals[i3] += n;
        if(primitiveGroup->mode == quads)
          vertexNormals[i4] += n;
      }
    }

    for(Normal* i = vertexNormals, * end = vertexNormals + verticesSize; i < end; ++i)
      if(i->length)
      {
        const float invLength = 1.f / static_cast<float>(i->length);
        i->x *= invLength;
        i->y *= invLength;
        i->z *= invLength;
      }
  }

  GraphicsContext::VertexBuffer<GraphicsContext::VertexPNT>* vertexBufferT = withTextureCoordinates ? graphicsContext.requestVertexBuffer<GraphicsContext::VertexPNT>() : nullptr;
  GraphicsContext::VertexBuffer<GraphicsContext::VertexPN>* vertexBuffer = withTextureCoordinates ? nullptr : graphicsContext.requestVertexBuffer<GraphicsContext::VertexPN>();
  if(withTextureCoordinates)
    vertexBufferT->vertices.reserve(verticesSize);
  else
    vertexBuffer->vertices.reserve(verticesSize);
  ASSERT(!withTextureCoordinates || texCoords->coords.size() == verticesSize);

  std::unordered_map<std::uint64_t, unsigned int> indexMap;

  auto getVertex = [this, vertexBuffer, vertexBufferT, withTextureCoordinates, &indexMap](std::list<unsigned int>::const_iterator& iter) -> unsigned int
  {
    const unsigned int vertexIndex = *(iter++);
    const unsigned int normalIndex = normalsDefined ? *(iter++) : vertexIndex;
    if(vertexIndex >= vertices->vertices.size() || normalIndex >= normals->normals.size())
      return 0; // Same as above: This does not make sense, but is better than crashing.
    const std::uint64_t combinedIndex = normalsDefined ? (vertexIndex | (static_cast<std::uint64_t>(normalIndex) << 32)) : static_cast<std::uint64_t>(vertexIndex);
    // Has this vertex already been added to the buffer?
    unsigned int& index = indexMap[combinedIndex];
    if(index)
      return index - 1;
    const Vertex& vertex = vertices->vertices[vertexIndex];
    const Normal& normal = normals->normals[normalIndex];
    if(withTextureCoordinates)
    {
      const TexCoord& texCoord = texCoords->coords[vertexIndex];
      vertexBufferT->vertices.emplace_back(Vector3f(vertex.x, vertex.y, vertex.z), Vector3f(normal.x, normal.y, normal.z), Vector2f(texCoord.x, texCoord.y));
      index = static_cast<unsigned int>(vertexBufferT->vertices.size());
    }
    else
    {
      vertexBuffer->vertices.emplace_back(Vector3f(vertex.x, vertex.y, vertex.z), Vector3f(normal.x, normal.y, normal.z));
      index = static_cast<unsigned int>(vertexBuffer->vertices.size());
    }
    return index - 1;
  };

  GraphicsContext::IndexBuffer* indexBuffer = graphicsContext.requestIndexBuffer();
  auto& indices = indexBuffer->indices;
  for(const PrimitiveGroup* primitiveGroup : primitiveGroups)
  {
    ASSERT(primitiveGroup->mode == triangles || primitiveGroup->mode == quads);
    ASSERT(primitiveGroup->vertices.size() % (normalsDefined ? (primitiveGroup->mode == triangles ? 6 : 8) : (primitiveGroup->mode == triangles ? 3 : 4)) == 0);
    for(auto iter = primitiveGroup->vertices.begin(), end = primitiveGroup->vertices.end(); iter != end;)
    {
      const auto i1 = getVertex(iter);
      const auto i2 = getVertex(iter);
      const auto i3 = getVertex(iter);

      indices.push_back(i1);
      indices.push_back(i2);
      indices.push_back(i3);

      if(primitiveGroup->mode == quads)
      {
        const auto i4 = getVertex(iter);
        indices.push_back(i3);
        indices.push_back(i4);
        indices.push_back(i1);
      }
    }
  }

  if(withTextureCoordinates)
  {
    vertexBufferT->vertices.shrink_to_fit();
    vertexBufferT->finish();
  }
  else
  {
    vertexBuffer->vertices.shrink_to_fit();
    vertexBuffer->finish();
  }

  GraphicsContext::Mesh* mesh = graphicsContext.requestMesh(withTextureCoordinates ? static_cast<GraphicsContext::VertexBufferBase*>(vertexBufferT) : vertexBuffer, indexBuffer, GraphicsContext::triangleList);

  meshCache[descriptor] = mesh;

  return mesh;
}
