#include "Game.h"
#include "Components.h"

void Game::Initialize()
{
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "AE-Asteroids" );
	render.Initialize( &window );
	input.Initialize( &window );
	file.Initialize( "data", "johnhues", "AE-Asteroids" );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	
	shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	shader.SetDepthTest( true );
	shader.SetDepthWrite( true );
	
	shipModel.Initialize( &file, "ship.fbx" );
	asteroidModel.Initialize( kAsteroidVerts, kAsteroidIndices, countof(kAsteroidVerts), countof(kAsteroidIndices) );
}

void Game::Terminate()
{
	AE_LOG( "Terminate" );
	//input.Terminate();
	render.Terminate();
	window.Terminate();
}

void Game::Load()
{
	// Ship
	{
		entt::entity entity = registry.create();
		
		registry.emplace< Transform >( entity );
		registry.emplace< Collision >( entity );
		
		Physics& physics = registry.emplace< Physics >( entity );
		physics.moveDrag = 0.7f;
		physics.rotationDrag = 3.0f;
		
		Ship& ship = registry.emplace< Ship >( entity );
		ship.local = true;
		ship.speed = 10.0f;
		ship.rotationSpeed = 10.0f;
		
		Model& model = registry.emplace< Model >( entity );
		model.mesh = &shipModel;
		model.shader = &shader;
		
		localShip = entity;
	}
	
	// Camera
	{
		entt::entity entity = registry.create();
		Transform& transform = registry.emplace< Transform >( entity );
		transform.SetPosition( ae::Vec3( 0, 0, 20.0f ) );
		registry.emplace< Camera >( entity );
	}

	// Asteroids
	for ( uint32_t i = 0; i < 8; i++ )
	{
		entt::entity entity = registry.create();
		
		Transform& transform = registry.emplace< Transform >( entity );
		transform.SetPosition( ae::Vec3( ae::Random( -1.0f, 1.0f ), ae::Random( -1.0f, 1.0f ), 0.0f ) );
		
		registry.emplace< Collision >( entity );
		
		float angle = ae::Random( 0.0f, ae::TWO_PI );
		float speed = ae::Random( 0.1f, 0.7f );
		Physics& physics = registry.emplace< Physics >( entity );
		physics.vel = ae::Vec3( cosf( angle ), sinf( angle ), 0.0f ) * speed;
		
		registry.emplace< Asteroid >( entity );
		
		Model& model = registry.emplace< Model >( entity );
		model.mesh = &asteroidModel;
		model.shader = &shader;
	}
}

void Game::Run()
{
	AE_LOG( "Run" );
	while ( !input.quit )
	//while ( !input.GetState()->exit )
	{
		input.Pump();
		
		// Update
		for( auto [ entity, ship, transform, physics ] : registry.view< Ship, Transform, Physics >().each() )
		{
			ship.Update( this, entity, transform, physics );
		}
		for( auto [ entity, asteroid, transform, physics ] : registry.view< Asteroid, Transform, Physics >().each() )
		{
			asteroid.Update( this, transform, physics );
		}
		for( auto [ entity, projectile ] : registry.view< Projectile >().each() )
		{
			projectile.Update( this, entity );
		}
		for( auto [ entity, physics, transform ]: registry.view< Physics, Transform >().each() )
		{
			physics.Update( this, transform );
		}
		for( auto [ entity, camera, transform ] : registry.view< Camera, Transform >().each() )
		{
			camera.Update( this, transform );
		}
		
		uint32_t pendingKillCount = m_pendingKill.Length();
		for ( uint32_t i = 0; i < pendingKillCount; i++ )
		{
			registry.destroy( m_pendingKill.GetKey( i ) );
		}
		m_pendingKill.Clear();
		
		// Render
		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple().ScaleRGB( 0.1f ) );
		
		auto drawView = registry.view< const Transform, const Model >();
		for( auto [ entity, transform, model ]: drawView.each() )
		{
			model.Draw( this, transform );
		}

		render.Present();
		timeStep.Wait();
	}
}

bool Game::IsOnScreen( ae::Vec3 pos ) const
{
	float halfWidth = render.GetAspectRatio();
	float halfHeight = 1.0f;
	if ( -halfWidth < pos.x && pos.x < halfWidth && -halfHeight < pos.y && pos.y < halfHeight )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Game::Kill( entt::entity entity )
{
	m_pendingKill.Set( entity, 0 );
}

entt::entity Game::SpawnProjectile( entt::entity source, ae::Vec3 offset )
{
	const Transform& sourceTransform = registry.get< Transform >( source );
	const Physics* sourcePhysics = registry.try_get< Physics >( source );
	
	entt::entity entity = registry.create();

	Transform& transform = registry.emplace< Transform >( entity );
	offset = ( sourceTransform.transform * ae::Vec4( offset, 0.0f ) ).GetXYZ();
	transform.SetPosition( sourceTransform.GetPosition() + offset );
	transform.transform.SetRotation( sourceTransform.transform.GetRotation() );
	transform.transform.SetScale( ae::Vec3( 0.3f ) );
	
	Physics& physics = registry.emplace< Physics >( entity );
	if ( sourcePhysics )
	{
		physics.vel += sourcePhysics->vel;
	}
	physics.vel += sourceTransform.GetForward() * 10.0f;
	
	Projectile& projectile = registry.emplace< Projectile >( entity );
	projectile.killTime = ae::GetTime() + 2.0f;

	Model& model = registry.emplace< Model >( entity );
	model.mesh = &shipModel;
	model.shader = &shader;
	
	return entity;
}
