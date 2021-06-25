#include "Game.h"
#include "Components.h"

ae::DebugLines*& GetDebugLines()
{
	static ae::DebugLines* g_debugLines = nullptr;
	return g_debugLines;
}

void Game::Initialize()
{
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "AE-Asteroids" );
	render.Initialize( &window );
	debugLines.Initialize( 256 );
	input.Initialize( &window );
	file.Initialize( "data", "johnhues", "AE-Asteroids" );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	GetDebugLines() = &debugLines;
	
	shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	shader.SetDepthTest( true );
	shader.SetDepthWrite( true );
	
	level0.Initialize( &file, "level0.fbx" );
	cubeModel.Initialize( &file, "cube.fbx" );
	shipModel.Initialize( &file, "ship.fbx" );
	asteroidModel.Initialize( kAsteroidVerts, kAsteroidIndices, countof(kAsteroidVerts), countof(kAsteroidIndices) );
}

void Game::Terminate()
{
	AE_INFO( "Terminate" );
	//input.Terminate();
	debugLines.Terminate();
	render.Terminate();
	window.Terminate();
}

void Game::Load()
{
	// Level
	{
		entt::entity entity = registry.create();
		
		Level& level = registry.emplace< Level >( entity );
		level.Clear();
		level.AddMesh( &level0, ae::Matrix4::Identity() );
		level.AddMesh( &cubeModel, ae::Matrix4::Scaling( ae::Vec3( 3.0f ) ) * ae::Matrix4::Translation( ae::Vec3( 3.0f, 3.0f, 0.0f ) ) );
	}
	
	// Ship
	{
		entt::entity entity = registry.create();
		
		registry.emplace< Transform >( entity );
		registry.emplace< Collision >( entity );
		
		Physics& physics = registry.emplace< Physics >( entity );
		physics.moveDrag = 0.7f;
		physics.rotationDrag = 1.7f;
		physics.collisionRadius = 0.7f;
		
		Ship& ship = registry.emplace< Ship >( entity );
		ship.local = true;
		ship.speed = 10.0f;
		ship.rotationSpeed = 5.0f;
		
		Team& team = registry.emplace< Team >( entity );
		team.teamId = TeamId::Player;
		
		registry.emplace< Shooter >( entity );
		
		Model& model = registry.emplace< Model >( entity );
		model.mesh = &shipModel;
		model.shader = &shader;
		model.color = ae::Color::PicoBlue();
		
		localShip = entity;
	}
	
	// Camera
	{
		entt::entity entity = registry.create();
		Transform& transform = registry.emplace< Transform >( entity );
		transform.SetPosition( ae::Vec3( 0, 0, 20.0f ) );
		registry.emplace< Camera >( entity );
	}

//	// Asteroids
//	for ( uint32_t i = 0; i < 8; i++ )
//	{
//		entt::entity entity = registry.create();
//
//		Transform& transform = registry.emplace< Transform >( entity );
//		transform.SetPosition( ae::Vec3( ae::Random( -1.0f, 1.0f ), ae::Random( -1.0f, 1.0f ), 0.0f ) );
//
//		registry.emplace< Collision >( entity );
//
//		float angle = ae::Random( 0.0f, ae::TWO_PI );
//		float speed = ae::Random( 0.1f, 0.7f );
//		Physics& physics = registry.emplace< Physics >( entity );
//		physics.vel = ae::Vec3( cosf( angle ), sinf( angle ), 0.0f ) * speed;
//
//		registry.emplace< Asteroid >( entity );
//
//		Model& model = registry.emplace< Model >( entity );
//		model.mesh = &asteroidModel;
//		model.shader = &shader;
//	}
	
	// Turret
	{
		entt::entity entity = registry.create();
		
		Transform& transform = registry.emplace< Transform >( entity );
		transform.SetPosition( ae::Vec3( -4.0f, 4.0f, 0.0f ) );
		
		registry.emplace< Collision >( entity );
		
		Physics& physics = registry.emplace< Physics >( entity );
		physics.rotationDrag = 1.7f;
		
		registry.emplace< Turret >( entity );
		
		Team& team = registry.emplace< Team >( entity );
		team.teamId = TeamId::Enemy;
		
		Shooter& shooter = registry.emplace< Shooter >( entity );
		shooter.fireInterval = 0.4f;
		
		Model& model = registry.emplace< Model >( entity );
		model.mesh = &shipModel;
		model.shader = &shader;
		model.color = ae::Color::PicoDarkPurple();
	}
}

void Game::Run()
{
	AE_INFO( "Run" );
	while ( !input.quit )
	//while ( !input.GetState()->exit )
	{
		input.Pump();
		
		// Update
		for( auto [ entity, ship, transform, physics ] : registry.view< Ship, Transform, Physics >().each() )
		{
			ship.Update( this, entity, transform, physics );
		}
		for( auto [ entity, turret ] : registry.view< Turret >().each() )
		{
			turret.Update( this, entity );
		}
		for( auto [ entity, asteroid, transform, physics ] : registry.view< Asteroid, Transform, Physics >().each() )
		{
			asteroid.Update( this, transform, physics );
		}
		for( auto [ entity, shooter ] : registry.view< Shooter >().each() )
		{
			shooter.Update( this, entity );
		}
		for( auto [ entity, projectile ] : registry.view< Projectile >().each() )
		{
			projectile.Update( this, entity );
		}
		for( auto [ entity, physics, transform ]: registry.view< Physics, Transform >().each() )
		{
			physics.Update( this, transform );
		}
		for( auto [ entity, level ]: registry.view< Level >().each() )
		{
			for( auto [ entity, physics, transform ]: registry.view< Physics, Transform >().each() )
			{
				if ( physics.collisionRadius )
				{
					level.Test( &transform, &physics );
				}
			}
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
		render.Clear( ae::Color::PicoBlack() );
		
		auto drawView = registry.view< const Transform, const Model >();
		for( auto [ entity, transform, model ]: drawView.each() )
		{
			model.Draw( this, transform );
		}
		
		if ( Level* level = registry.try_get< Level >( this->level ) )
		{
			level->Render( this );
		}
		
		//debugLines.Render( worldToNdc );
		debugLines.Clear();

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
	const Team& sourceTeam = registry.get< Team >( source );
	const Physics* sourcePhysics = registry.try_get< Physics >( source );
	
	entt::entity entity = registry.create();

	Transform& transform = registry.emplace< Transform >( entity );
	offset = ( sourceTransform.transform * ae::Vec4( offset, 0.0f ) ).GetXYZ();
	transform.SetPosition( sourceTransform.GetPosition() + offset );
	transform.transform.SetRotation( sourceTransform.transform.GetRotation() );
	transform.transform.SetScale( ae::Vec3( 0.25f ) );
	
	Physics& physics = registry.emplace< Physics >( entity );
	if ( sourcePhysics )
	{
		physics.vel += sourcePhysics->vel;
	}
	physics.vel += sourceTransform.GetForward() * 15.0f;
	physics.collisionRadius = 0.1f;
	
	Projectile& projectile = registry.emplace< Projectile >( entity );
	projectile.killTime = ae::GetTime() + 2.0f;
	
	Team& team = registry.emplace< Team >( entity );
	team.teamId = sourceTeam.teamId;

	Model& model = registry.emplace< Model >( entity );
	model.mesh = &shipModel;
	model.shader = &shader;
	switch ( sourceTeam.teamId )
	{
		case TeamId::None:
			model.color = ae::Color::Gray();
			break;
		case TeamId::Player:
			model.color = ae::Color::PicoYellow();
			break;
		case TeamId::Enemy:
			model.color = ae::Color::PicoRed();
			break;
		default:
			AE_FAIL_MSG( "Invalid team" );
			break;
	}
	
	return entity;
}
