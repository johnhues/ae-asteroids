#include "Level.h"
#include "Components.h"
#include "Game.h"
#include "Resources.h"

ae::Vec3 Level::Line::GetNormal() const
{
	ae::Vec3 n = ( p1 - p0 );
	std::swap( n.x, n.y );
	n.y = -n.y;
	n.SafeNormalize();
	return n;
}

bool LineSegmentPlaneIntersection( ae::Vec3 p, ae::Vec3 n, ae::Vec3 p0, ae::Vec3 p1, ae::Vec3* outP )
{
	ae::Vec3 ab = p1 - p0;
	n.SafeNormalize();
	float d = -n.Dot( p );
	float t = ( d - n.Dot( p0 ) ) / n.Dot( ab );
	if ( t < 0.0f || t > 1.0f )
	{
		return false;
	}

//	if ( tOut )
//	{
//	  *tOut = c;
//	}
	if ( outP )
	{
	  *outP = p0 + ab * t;
	}
	return true;
}

bool TrianglePlaneIntersection( ae::Vec3 p, ae::Vec3 n, const ae::Vec3* t, ae::Vec3* outP0, ae::Vec3* outP1 )
{
	int count = 0;
	ae::Vec3 r[ 3 ];
	count += LineSegmentPlaneIntersection( p, n, t[ 0 ], t[ 1 ], &r[ count ] ) ? 1 : 0;
	count += LineSegmentPlaneIntersection( p, n, t[ 1 ], t[ 2 ], &r[ count ] ) ? 1 : 0;
	count += LineSegmentPlaneIntersection( p, n, t[ 2 ], t[ 0 ], &r[ count ] ) ? 1 : 0;
	if ( count == 2 )
	{
		ae::Vec3 rn = ( r[ 1 ] - r[ 0 ] );
		rn = rn.RotateCopy( n, ae::HALF_PI );
		ae::Vec3 tn = ( t[ 1 ] - t[ 0 ] ).Cross( t[ 2 ] - t[ 0 ] );
		if ( tn.Dot( rn ) > 0.0f )
		{
			// Make sure winding order is preserved in 2d
			std::swap( r[ 0 ], r[ 1 ] );
		}
		*outP0 = r[ 0 ];
		*outP1 = r[ 1 ];
		return true;
	}
	return false;
}

void Level::AddMesh( const MeshResource* mesh, ae::Matrix4 localToWorld )
{
	LevelMesh& levelMesh = m_levelMeshes.Append( LevelMesh() );
	levelMesh.mesh = mesh;
	levelMesh.localToWorld = localToWorld;
	
	for ( const LevelMesh& levelMesh : m_levelMeshes )
	{
		uint32_t triCount = levelMesh.mesh->vertexData.GetIndexCount() / 3;
		AE_ASSERT( levelMesh.mesh->vertexData.GetIndexSize() == 2 );
		AE_ASSERT( sizeof(Vertex) == levelMesh.mesh->vertexData.GetVertexSize() );
		const uint16_t* indices = (const uint16_t*)levelMesh.mesh->vertexData.GetIndices();
		const Vertex* verts = (const Vertex*)levelMesh.mesh->vertexData.GetVertices();
		for ( uint32_t i = 0; i < triCount; i++ )
		{
			ae::Vec3 p0, p1;
			ae::Vec3 t[ 3 ];
			t[ 0 ] = ( levelMesh.localToWorld * verts[ indices[ i * 3 ] ].pos ).GetXYZ();
			t[ 1 ] = ( levelMesh.localToWorld * verts[ indices[ i * 3 + 1 ] ].pos ).GetXYZ();
			t[ 2 ] = ( levelMesh.localToWorld * verts[ indices[ i * 3 + 2 ] ].pos ).GetXYZ();
			if ( TrianglePlaneIntersection( ae::Vec3( 0.0f ), ae::Vec3( 0,0,1 ), t, &p0, &p1 ) )
			{
				m_collision.Append( { p0, p1 } );
			}
		}
	}
}

bool Level::Test( Transform* transform, Physics* physics )
{
	bool hit = false;
	float closestDistance = ae::MaxValue< float >();
	ae::Vec3 closest;
	ae::Vec3 closestNormal;
	
	ae::DebugLines* debugLines = GetDebugLines();
	ae::Vec3 pos = transform->GetPosition();
	for ( const Line& l : m_collision )
	{
		// project onto line
		ae::Vec3 D = ( l.p1 - l.p0 );
		float len = D.SafeNormalize();
		float d = ( pos - l.p0 ).Dot( D );
		d = ae::Clip( d, 0.0f, len );
		ae::Vec3 r = l.p0 + D * d;
		
		float cDist = ( r - pos ).Length();
		if ( cDist < physics->collisionRadius && cDist < closestDistance )
		{
			closestNormal = l.GetNormal();
			closest = r;
			closestDistance = cDist;
			hit = true;
		}
	}
	
	if ( hit )
	{
		ae::Vec3 outer = pos + ( closest - pos ).SafeNormalizeCopy() * physics->collisionRadius;
		GetDebugLines()->AddSphere( closest, 0.1f, ae::Color::Red(), 8 );
		GetDebugLines()->AddSphere( outer, 0.1f, ae::Color::Green(), 8 );
		GetDebugLines()->AddSphere( pos + ( closest - outer ), 0.1f, ae::Color::Blue(), 8 );
		transform->SetPosition( pos + ( closest - outer ) );
		
		physics->vel.ZeroDirection( -closestNormal );
	}
	GetDebugLines()->AddLine( pos, pos + physics->vel, ae::Color::Green() );
	
	return hit;
}

void Level::Render( Game* game )
{
	for ( const LevelMesh& levelMesh : m_levelMeshes )
	{
		ae::Matrix4 normalMatrix = levelMesh.localToWorld.GetNormalMatrix();
		ae::UniformList uniformList;
		uniformList.Set( "u_modelToNdc", game->worldToNdc * levelMesh.localToWorld );
		uniformList.Set( "u_normalMatrix", normalMatrix );
		uniformList.Set( "u_ambientLight", game->ambientLight.GetLinearRGB() );
		uniformList.Set( "u_color", ae::Color::Gray().GetLinearRGB() );
		levelMesh.mesh->vertexData.Render( &game->shader, uniformList );
		
		for ( const Line& l : m_collision )
		{
			ae::Vec3 n = l.GetNormal();
			ae::Vec3 c = ( l.p0 + l.p1 ) * 0.5f;
			game->debugLines.AddLine( l.p0, l.p1, ae::Color::Red() );
			game->debugLines.AddLine( c, c + n, ae::Color::Red() );
		}
	}
}

void Level::Clear()
{
	m_levelMeshes.Clear();
	m_collision.Clear();
}
