#include "Resources.h"
#include "Game.h"
#include "ofbx.h"

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
ae::Matrix4 ofbxToAe( const ofbx::Matrix& m )
{
	ae::Matrix4 result;
	for ( uint32_t i = 0; i < 16; i++ )
	{
		result.d[ i ] = m.m[ i ];
	}
	return result;
}

//------------------------------------------------------------------------------
// Shaders
//------------------------------------------------------------------------------
const char* kVertShader = "\
	AE_UNIFORM mat4 u_modelToNdc;\
	AE_UNIFORM mat4 u_normalMatrix;\
	AE_IN_HIGHP vec4 a_position;\
	AE_IN_HIGHP vec4 a_color;\
	AE_IN_HIGHP vec4 a_normal;\
	AE_OUT_HIGHP vec4 v_color;\
	AE_OUT_HIGHP vec4 v_normal;\
	void main()\
	{\
		v_color = a_color;\
		v_normal = u_normalMatrix * a_normal;\
		gl_Position = u_modelToNdc * a_position;\
	}";

const char* kFragShader = "\
	AE_UNIFORM vec3 u_ambientLight;\
	AE_UNIFORM vec3 u_color;\
	AE_IN_HIGHP vec4 v_color;\
	AE_IN_HIGHP vec4 v_normal;\
	void main()\
	{\
		float d = max(0.0, dot(v_normal.rgb, normalize(vec3(1,1,1))));\
		vec3 light = vec3(d) + u_ambientLight;\
		vec3 n = normalize(v_normal.rgb);\
		AE_COLOR = vec4(u_color * v_color.rgb * light, 1.0);\
	}";

//------------------------------------------------------------------------------
// Ship
//------------------------------------------------------------------------------
const Vertex kTriangleVerts[] =
{
	{ ae::Vec4( -0.2f, -0.5f, 0.0f, 1.0f ), ae::Vec4( 0.0f, 0.0f, 1.0f, 0.0f ), ae::Color::PicoRed().GetLinearRGBA() },
	{ ae::Vec4( 0.2f, -0.5f, 0.0f, 1.0f ), ae::Vec4( 0.0f, 0.0f, 1.0f, 0.0f ), ae::Color::PicoGreen().GetLinearRGBA() },
	{ ae::Vec4( 0.0f, 0.5f, 0.0f, 1.0f ), ae::Vec4( 0.0f, 0.0f, 1.0f, 0.0f ), ae::Color::PicoBlue().GetLinearRGBA() },
};
const uint16_t kTriangleIndices[] =
{
	0, 1, 2
};

//------------------------------------------------------------------------------
// Asteroid
//------------------------------------------------------------------------------
const Vertex kAsteroidVerts[] =
{
	{ ae::Vec4( -0.5f, -0.5f, 0.0f, 1.0f ), ae::Vec4( 0.0f, 0.0f, 1.0f, 0.0f ), ae::Color::PicoDarkGray().GetLinearRGBA() },
	{ ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ), ae::Vec4( 0.0f, 0.0f, 1.0f, 0.0f ), ae::Color::PicoDarkGray().GetLinearRGBA() },
	{ ae::Vec4( 0.5f, 0.5f, 0.0f, 1.0f ), ae::Vec4( 0.0f, 0.0f, 1.0f, 0.0f ), ae::Color::PicoDarkGray().GetLinearRGBA() },
	{ ae::Vec4( -0.5f, 0.5f, 0.0f, 1.0f ), ae::Vec4( 0.0f, 0.0f, 1.0f, 0.0f ), ae::Color::PicoDarkGray().GetLinearRGBA() },
};
const uint16_t kAsteroidIndices[] =
{
	0, 1, 2,
	0, 2, 3,
};

//------------------------------------------------------------------------------
// MeshResource member functions
//------------------------------------------------------------------------------
void MeshResource::Initialize( const Vertex* vertices, const uint16_t* indices, uint32_t vertexCount, uint32_t indexCount )
{
	vertexData.Initialize( sizeof(*vertices), sizeof(*indices), vertexCount, indexCount, ae::VertexData::Primitive::Triangle, ae::VertexData::Usage::Static, ae::VertexData::Usage::Static );
	vertexData.AddAttribute( "a_position", 4, ae::VertexData::Type::Float, offsetof( Vertex, pos ) );
	vertexData.AddAttribute( "a_normal", 4, ae::VertexData::Type::Float, offsetof( Vertex, normal ) );
	vertexData.AddAttribute( "a_color", 4, ae::VertexData::Type::Float, offsetof( Vertex, color ) );
	vertexData.SetVertices( vertices, vertexCount );
	vertexData.SetIndices( indices, indexCount );
}

void MeshResource::Initialize( ae::FileSystem* file, const char* filePath )
{
	auto errFn = [&]()
	{
		ae::Str256 rootPath;
		file->GetRootDir( ae::FileSystem::Root::Data, &rootPath );
		ae::FileSystem::AppendToPath( &rootPath, filePath );
		AE_ERR( "Could not load mesh '#'", rootPath );
	};
	uint32_t fileSize = file->GetSize( ae::FileSystem::Root::Data, filePath );
	if ( !fileSize )
	{
		errFn();
		return;
	}
	ae::Scratch< uint8_t > fileData( TAG_RESOURCE, fileSize );
	if ( fileSize != file->Read( ae::FileSystem::Root::Data, filePath, fileData.Data(), fileSize ) )
	{
		errFn();
		return;
	}
	ofbx::IScene* scene = ofbx::load( (ofbx::u8*)fileData.Data(), fileSize, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE );
	if ( scene )
	{
		uint32_t meshCount = scene->getMeshCount();
		
		uint32_t totalVerts = 0;
		uint32_t totalIndices = 0;
		for ( uint32_t i = 0; i < meshCount; i++ )
		{
			const ofbx::Mesh* mesh = scene->getMesh( i );
			const ofbx::Geometry* geo = mesh->getGeometry();
			totalVerts += geo->getVertexCount();
			totalIndices += geo->getIndexCount();
		}
		
		uint32_t indexOffset = 0;
		ae::Array< Vertex > vertices( TAG_RESOURCE, totalVerts );
		ae::Array< uint16_t > indices( TAG_RESOURCE, totalIndices );
		for ( uint32_t i = 0; i < meshCount; i++ )
		{
			const ofbx::Mesh* mesh = scene->getMesh( i );
			const ofbx::Geometry* geo = mesh->getGeometry();
			ae::Matrix4 localToWorld = ofbxToAe( mesh->getGlobalTransform() );
			ae::Matrix4 normalMatrix = localToWorld.GetNormalMatrix();
			
			uint32_t vertexCount = geo->getVertexCount();
			const ofbx::Vec3* meshVerts = geo->getVertices();
			const ofbx::Vec3* meshNormals = geo->getNormals();
			for ( uint32_t j = 0; j < vertexCount; j++ )
			{
				ofbx::Vec3 p = meshVerts[ j ];
				Vertex v;
				v.pos.x = p.x;
				v.pos.y = p.y;
				v.pos.z = p.z;
				v.pos.w = 1.0f;
				v.pos = localToWorld * v.pos;
				v.normal = ae::Vec4( 0.0f );
				v.color = ae::Color::Gray().GetLinearRGBA();
				vertices.Append( v );
			}
			
			uint32_t indexCount = geo->getIndexCount();
			const int32_t* meshIndices = geo->getFaceIndices();
			for ( uint32_t j = 0; j < indexCount; j++ )
			{
				int32_t index = ( meshIndices[ j ] < 0 ) ? ( -meshIndices[ j ] - 1 ) : meshIndices[ j ];
				AE_ASSERT( index < vertexCount );
				index += indexOffset;
				indices.Append( index );
				
				ofbx::Vec3 n = meshNormals[ j ];
				Vertex& v = vertices[ index ];
				v.normal.x = n.x;
				v.normal.y = n.y;
				v.normal.z = n.z;
				v.normal.w = 0.0f;
				v.normal = normalMatrix * v.normal;
				v.normal.SafeNormalize();
			}
			
			indexOffset += vertexCount;
		}
		Initialize( vertices.Begin(), indices.Begin(), vertices.Length(), indices.Length() );
	}
}
