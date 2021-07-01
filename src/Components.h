#ifndef ASTEROIDS_COMPONENTS_H
#define ASTEROIDS_COMPONENTS_H

#include "ae/aether.h"
#include "Game.h"
#include "Component.h"

struct Transform : public Component
{
	ae::Matrix4 transform = ae::Matrix4::Identity();
	
	void SetPosition( ae::Vec3 pos );
	ae::Vec3 GetPosition() const;
	ae::Vec3 GetForward() const { return transform.GetAxis( 1 ).SafeNormalizeCopy(); }
	ae::Vec3 GetRight() const { return transform.GetAxis( 0 ).SafeNormalizeCopy(); }
	float GetYaw() const;
	float GetRoll() const;
};

struct Physics : public Component
{
	void Update( class Game* game, Transform& transform );
	
	float GetSpeed() const { return vel.Length(); }
	
	float moveDrag = 0.0f;
	float rotationDrag = 0.0f;

	ae::Vec3 accel = ae::Vec3( 0.0f );
	ae::Vec3 vel = ae::Vec3( 0.0f );
	float rotationVel = 0.0f;
	
	float collisionRadius = 0.0f;
	bool hit = false;
};

struct Ship : public Component
{
	void Update( class Game* game, entt::entity entity, Transform& transform, Physics& physics );
	
	bool local = false;
	float speed = 10.0f;
	float rotationSpeed = 10.0f;
};

struct Shooter : public Component
{
	void Update( class Game* game, entt::entity entity );
	
	bool fire = false;
	float fireInterval = 0.25f;
	
private:
	double m_lastFired = 0.0;
};

struct Camera : public Component
{
	void Update( class Game* game, Transform& transform );
	
	float posSnappiness = 0.5f;
	float zoomSnappiness = 0.05f;
};

struct Asteroid : public Component
{
	void Update( class Game* game, Transform& transform, Physics& physics );
	
	int32_t dummy;
};

struct Turret : public Component
{
	void Update( class Game* game, entt::entity entity );
	
	float range = 10.0f;
};

struct Projectile : public Component
{
	void Update( class Game* game, entt::entity entity );
	
	double killTime = 0.0;
};

struct Team : public Component
{
	TeamId teamId = TeamId::None;
};

struct Collision : public Component
{
	int32_t dummy;
};

struct Model : public Component
{
	void Draw( class Game* game, const Transform& transform ) const;
	
	const class MeshResource* mesh = nullptr;
	const ae::Shader* shader = nullptr;
	ae::Color color = ae::Color::White();
};

#endif
