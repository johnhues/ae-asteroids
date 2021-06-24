#ifndef ASTEROIDS_COMPONENTS_H
#define ASTEROIDS_COMPONENTS_H

#include "ae/aether.h"
#include "entt/entt.hpp"

struct Transform
{
	ae::Matrix4 transform = ae::Matrix4::Identity();
	
	void SetPosition( ae::Vec3 pos );
	ae::Vec3 GetPosition() const;
	ae::Vec3 GetForward() const { return transform.GetAxis( 1 ).SafeNormalizeCopy(); }
	ae::Vec3 GetRight() const { return transform.GetAxis( 0 ).SafeNormalizeCopy(); }
	float GetYaw() const;
	float GetRoll() const;
};

struct Physics
{
	void Update( class Game* game, Transform& transform );
	
	float GetSpeed() const { return vel.Length(); }
	
	float moveDrag = 0.0f;
	float rotationDrag = 0.0f;

	ae::Vec3 accel = ae::Vec3( 0.0f );
	ae::Vec3 vel = ae::Vec3( 0.0f );
	float rotationVel = 0.0f;
};

struct Ship
{
	void Update( class Game* game, entt::entity entity, Transform& transform, Physics& physics );
	
	bool local = false;
	float speed = 10.0f;
	float rotationSpeed = 10.0f;
	double lastFired = 0.0;
};

struct Camera
{
	void Update( class Game* game, Transform& transform );
	
	float posSnappiness = 0.5f;
	float zoomSnappiness = 0.1f;
};

struct Asteroid
{
	void Update( class Game* game, Transform& transform, Physics& physics );
	
	int32_t dummy;
};

struct Projectile
{
	void Update( class Game* game, entt::entity entity );
	
	double killTime = 0.0;
};

struct Collision
{
	int32_t dummy;
};

struct Model
{
	void Draw( class Game* game, const Transform& transform ) const;
	
	const class MeshResource* mesh;
	const ae::Shader* shader;
};

#endif
