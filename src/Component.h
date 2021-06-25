#ifndef ASTEROIDS_OBJECT_H
#define ASTEROIDS_OBJECT_H

#include "entt/entt.hpp"

struct Component
{
	virtual ~Component() {}
	virtual void OnHit( entt::entity entity ) {}
	entt::registry* registry;
};

#endif
