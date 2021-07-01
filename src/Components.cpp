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
	if ( input.Get( ae::Key::Up ) )
	{
		physics.accel.y += 1.0f;
	}
	if ( input.Get( ae::Key::Down ) )
	{
		physics.accel.y -= 1.0f;
	}
	physics.accel.AddRotationXY( transform.GetYaw() );
	physics.accel.SafeNormalize();
	physics.accel *= speed;
	
	if ( input.Get( ae::Key::Left ) )
	{
		physics.rotationVel += rotationSpeed * dt;
	}
	if ( input.Get( ae::Key::Right ) )
	{
		physics.rotationVel -= rotationSpeed * dt;
	}
	
	Shooter& shooter = game->registry.get< Shooter >( entity );
	shooter.fire = input.Get( ae::Key::Space );
}

void Shooter::Update( Game* game, entt::entity entity )
{
	if ( !fire )
	{
		return;
	}
	
	double currentTime = ae::GetTime();
	if ( m_lastFired + fireInterval < currentTime )
	{
		game->SpawnProjectile( entity, ae::Vec3( 0.6f, 0.3f, 0.0f ) );
		game->SpawnProjectile( entity, ae::Vec3( -0.6f, 0.3f, 0.0f ) );
		m_lastFired = currentTime;
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
		
		float targetZoom = 25.0f;
		if ( shipSpeed < 2.0f )
		{
			targetZoom = 18.0f;
		}
		camPos.z = ae::DtLerp( camPosPrev.z, zoomSnappiness, dt, targetZoom );
	}
	
//	camPos.x = 0.0f;
//	camPos.y = ae::Max( camPos.y, -4.0f );
	
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

void Turret::Update( Game* game, entt::entity entity )
{
	float dt = game->timeStep.GetDt();
	Transform& transform = game->registry.get< Transform >( entity );
	Physics& physics = game->registry.get< Physics >( entity );
	TeamId teamId = game->registry.get< Team >( entity ).teamId;
	
	float rangeSq = range * range;
	
	float targetDistanceSq = ae::MaxValue< float >();
	Transform* targetTransform = nullptr;
	for( auto [ entity, ship, shipTransform, shipTeam ] : game->registry.view< Ship, Transform, Team >().each() )
	{
		if ( shipTeam.teamId != teamId )
		{
			float distanceSq = ( shipTransform.GetPosition() - transform.GetPosition() ).LengthSquared();
			game->debugLines.AddDistanceCheck( shipTransform.GetPosition(), transform.GetPosition(), range );
			if ( distanceSq <= rangeSq && distanceSq < targetDistanceSq )
			{
				targetTransform = &shipTransform;
				targetDistanceSq = distanceSq;
			}
		}
	}
	
	Shooter& shooter = game->registry.get< Shooter >( entity );
	shooter.fire = false;
	if ( targetTransform )
	{
		ae::Vec2 forward = transform.GetForward().GetXY().SafeNormalizeCopy();
		ae::Vec2 diff = ( targetTransform->GetPosition() - transform.GetPosition() ).GetXY().SafeNormalizeCopy();
		ae::Vec2 n( -forward.y, forward.x );
		float d = diff.Dot( n );
		if ( d > 0.0f )
		{
			physics.rotationVel += dt;
		}
		else if ( d < 0.0f )
		{
			physics.rotationVel -= dt;
		}
		
		if ( forward.Dot( diff ) > 0.4f )
		{
			shooter.fire = true;
		}
	}
}

void Projectile::Update( Game* game, entt::entity entity )
{
	if ( killTime && killTime < ae::GetTime() )
	{
		game->Kill( entity );
	}
	
	Physics& physics = game->registry.get< Physics >( entity );
	if ( physics.hit )
	{
		game->Kill( entity );
	}
}

void Model::Draw( Game* game, const Transform& transform ) const
{
	ae::Matrix4 normalMatrix = transform.transform.GetNormalMatrix();
	ae::UniformList uniformList;
	uniformList.Set( "u_modelToNdc", transform.transform * game->worldToNdc );
	uniformList.Set( "u_normalMatrix", normalMatrix );
	uniformList.Set( "u_ambientLight", game->ambientLight.GetLinearRGB() );
	uniformList.Set( "u_color", color.GetLinearRGB() );
	mesh->vertexData.Render( shader, uniformList );
}
