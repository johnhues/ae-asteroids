#ifndef ASTEROIDS_GAME_H
#define ASTEROIDS_GAME_H

#include "ae/aether.h"
#include "entt/entt.hpp"
#include "Resources.h"

const ae::Tag TAG_GAME = "game";
const ae::Tag TAG_RESOURCE = "resource";

class Game
{
public:
	void Initialize();
	void Terminate();
	void Load();
	void Run();
	
	bool IsOnScreen( ae::Vec3 pos ) const;
	
	void Kill( entt::entity entity );
	entt::entity SpawnProjectile( entt::entity entity, ae::Vec3 offset );
	
	// Systems
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::FileSystem file;
	ae::TimeStep timeStep;
	entt::registry registry;
	
	// Game state
	entt::entity localShip = entt::entity();
	ae::Matrix4 worldToNdc = ae::Matrix4::Identity();
	
	// Resources
	ae::Shader shader;
	MeshResource shipModel;
	MeshResource asteroidModel;
	
private:
	ae::Map< entt::entity, int > m_pendingKill = TAG_GAME;
};

#endif
