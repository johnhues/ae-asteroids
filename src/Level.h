#ifndef ASTEROIDS_LEVEL_H
#define ASTEROIDS_LEVEL_H

#include "ae/aether.h"
#include "Component.h"

const ae::Tag TAG_LEVEL = "level";

class Level : public Component
{
public:
	void AddMesh( const class MeshResource* mesh, ae::Matrix4 localToWorld );
	bool Test( class Transform* transform, class Physics* physics );
	void Render( class Game* game );
	void Clear();
	
private:
	struct LevelMesh
	{
		const MeshResource* mesh;
		ae::Matrix4 localToWorld;
	};
	struct Line
	{
		ae::Vec3 GetNormal() const;
		ae::Vec3 p0;
		ae::Vec3 p1;
	};
	ae::Array< LevelMesh > m_levelMeshes = TAG_LEVEL;
	ae::Array< Line > m_collision = TAG_LEVEL;
};

#endif
