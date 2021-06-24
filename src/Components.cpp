#include "Components.h"
#include "Resources.h"
#include "Game.h"

void Transform::SetPosition( ae::Vec3 pos )
{
	transform.SetTranslation( pos );
}

ae::Vec3 Transform::GetPosition() const
{
	return transform.GetTranslation();
}

float Transform::GetYaw() const
{
	ae::Vec3 facing = transform.GetRotation().GetDirectionXY();
	return atan2( facing.y, facing.x );
}

float Transform::GetRoll() const
{
	return 0.0f;
}

void Physics::Update( Game* game, Transform& transform )
{
	const float dt = game->timeStep.GetDt();
	
	static float s_roll = 0.0f;
	s_roll += rotationDrag * dt * 0.25f;
	
	ae::Vec3 pos = transform.GetPosition();
	ae::Vec3 scale = transform.transform.GetScale();
	float yaw = transform.GetYaw();
	float roll = s_roll;//transform.GetRoll();

	vel += accel * dt;
	pos += vel * dt;
	
	float speed = vel.Length();
	speed = ae::DtLerp( speed, moveDrag, dt, 0.0f );
	vel = vel.SafeNormalizeCopy() * speed;
	
	yaw += rotationVel * dt;
	rotationVel = ae::DtLerp( rotationVel, rotationDrag, dt, 0.0f );

	transform.transform = ae::Matrix4::Scaling( scale );
//	transform.transform *= ae::Matrix4::RotationY( roll );
	transform.transform *= ae::Matrix4::RotationZ( yaw );
	transform.transform *= ae::Matrix4::Translation( pos );
}


void Ship::Update( Game* game, entt::entity entity, Transform& transform, Physics& physics )
{
	if ( !local )
	{
		return;
	}
	
	const float dt = game->timeStep.GetDt();
	const ae::Input& input = game->input;
	
	physics.accel = ae::Vec3( 0.0f );
	if ( input.Get( ae::Key::Up ) || input.Get( ae::Key::W ) )
	{
		physics.accel.y += 1.0f;
	}
	if ( input.Get( ae::Key::Down ) || input.Get( ae::Key::S ) )
	{
		physics.accel.y -= 1.0f;
	}
	physics.accel.AddRotationXY( transform.GetYaw() );
	physics.accel.SafeNormalize();
	physics.accel *= speed;
	
	if ( input.Get( ae::Key::Left ) || input.Get( ae::Key::A ) )
	{
		physics.rotationVel += rotationSpeed * dt;
	}
	if ( input.Get( ae::Key::Right ) || input.Get( ae::Key::D ) )
	{
		physics.rotationVel -= rotationSpeed * dt;
	}
	
	float fireInterval = 0.35f;
	double currentTime = ae::GetTime();
	if ( lastFired + fireInterval < currentTime && input.Get( ae::Key::Space ) )
	{
		game->SpawnProjectile( entity, ae::Vec3( 0.6f, 0.3f, 0.0f ) );
		game->SpawnProjectile( entity, ae::Vec3( -0.6f, 0.3f, 0.0f ) );
		lastFired = currentTime;
	}
}

void Camera::Update( Game* game, Transform& transform )
{
	const float dt = game->timeStep.GetDt();
	const ae::Vec3 camPosPrev = transform.GetPosition();
	ae::Vec3 camPos = camPosPrev;
	if ( Transform* shipTransform = game->registry.try_get< Transform >( game->localShip ) )
	{
		Physics& shipPhysics = game->registry.get< Physics >( game->localShip );
		float shipSpeed = shipPhysics.GetSpeed();
		
		ae::Vec3 targetPos = shipTransform->GetPosition();
		targetPos += shipPhysics.vel * 1.75f;
		targetPos += shipTransform->GetRight() * shipPhysics.rotationVel * shipSpeed * -0.25f;
		camPos = ae::DtLerp( camPosPrev, posSnappiness, dt, targetPos );
		
		float targetZoom = 20.0f;
		if ( shipSpeed < 2.0f )
		{
			targetZoom = 10.0f;
		}
		camPos.z = ae::DtLerp( camPosPrev.z, zoomSnappiness, dt, targetZoom );
	}
	
	transform.SetPosition( camPos );
	game->worldToNdc = ae::Matrix4::WorldToView( camPos, ae::Vec3( 0, 0, -1 ), ae::Vec3( 0, 1, 0 ) );
	game->worldToNdc *= ae::Matrix4::ViewToProjection( 0.9f, game->render.GetAspectRatio(), 1.0f, 100.0f );
}

void Asteroid::Update( Game* game, Transform& transform, Physics& physics )
{
	if ( !game->IsOnScreen( transform.GetPosition() ) )
	{
		transform.SetPosition( ae::Vec3( ae::Random( -1.0f, 1.0f ), ae::Random( -1.0f, 1.0f ), 0.0f ) );
		
		float angle = ae::Random( 0.0f, ae::TWO_PI );
		float speed = ae::Random( 0.1f, 0.7f );
		physics.vel = ae::Vec3( cosf( angle ), sinf( angle ), 0.0f ) * speed;
	}
}

void Projectile::Update( Game* game, entt::entity entity )
{
	if ( killTime && killTime < ae::GetTime() )
	{
		game->Kill( entity );
	}
}

void Model::Draw( Game* game, const Transform& transform ) const
{
	ae::Matrix4 normalMatrix = transform.transform.GetNormalMatrix();
	mesh->Draw( shader, normalMatrix, transform.transform * game->worldToNdc );
}
