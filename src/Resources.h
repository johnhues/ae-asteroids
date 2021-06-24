#ifndef ASTEROIDS_RESOURCES_H
#define ASTEROIDS_RESOURCES_H

#include "ae/aether.h"

//------------------------------------------------------------------------------
// Vertex
//------------------------------------------------------------------------------
struct Vertex
{
	ae::Vec4 pos;
	ae::Vec4 normal;
	ae::Vec4 color;
};

//------------------------------------------------------------------------------
// Shaders
//------------------------------------------------------------------------------
extern const char* kVertShader;
extern const char* kFragShader;

//------------------------------------------------------------------------------
// Ship
//------------------------------------------------------------------------------
extern const Vertex kTriangleVerts[ 3 ];
extern const uint16_t kTriangleIndices[ 3 ];

//------------------------------------------------------------------------------
// Asteroid
//------------------------------------------------------------------------------
extern const Vertex kAsteroidVerts[ 4 ];
extern const uint16_t kAsteroidIndices[ 6 ];

//------------------------------------------------------------------------------
// MeshResource class
//------------------------------------------------------------------------------
class MeshResource
{
public:
	void Initialize( const Vertex* vertices, const uint16_t* indices, uint32_t vertexCount, uint32_t indexCount );
	void Initialize( ae::FileSystem* file, const char* filePath );
	void Draw( const ae::Shader* shader, const ae::Matrix4& normalMatrix, const ae::Matrix4& localToNdc ) const;
	
private:
	ae::VertexData m_vertexData;
};

#endif
