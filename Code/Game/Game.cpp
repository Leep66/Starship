#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/App.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Bullet.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Debris.hpp"
#include "Game/Beetle.hpp"
#include "Game/Wasp.hpp"
#include "Game/Star.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Window/Window.hpp"

extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern DevConsole* g_theDevConsole;
extern EventSystem* g_theEventSystem;
extern BitmapFont* g_theFont;
extern Window* g_theWindow;



Game::Game(App* owner)
	: m_App(owner)
{
	Startup();
}

Game::~Game()
{
	
	g_theAudio->StopSound(m_musicPlayback);
	for (int asIndex = 0; asIndex < MAX_ASTEROIDS; ++asIndex)
	{
		delete m_asteroids[asIndex];
		m_asteroids[asIndex] = nullptr;
	}

	for (int bulletIndex = 0; bulletIndex < MAX_BULLETS; ++bulletIndex)
	{
		delete m_bullets[bulletIndex];
		m_bullets[bulletIndex] = nullptr;
	}

	for (int beetIndex = 0; beetIndex < MAX_BETTLES; ++beetIndex)
	{
		delete m_beetles[beetIndex];
		m_beetles[beetIndex] = nullptr;
	}

	for (int waspIndex = 0; waspIndex < MAX_WASPS; ++waspIndex)
	{
		delete m_wasps[waspIndex];
		m_wasps[waspIndex] = nullptr;
	}

	for (int starIndex = 0; starIndex < MAX_STARS; ++starIndex)
	{
		delete m_stars[starIndex];
		m_stars[starIndex] = nullptr;
	}

}

void Game::Startup()
{
	m_clock = new Clock();
	m_playerShipA = new PlayerShip(this, Vec2(WORLD_CENTER_X - 50.f, WORLD_CENTER_Y), 0.f, Rgba8(102, 153, 204, 255), false);
	m_playerShipB = new PlayerShip(this, Vec2(WORLD_CENTER_X + 50.f, WORLD_CENTER_Y), 180.f, Rgba8(153, 0, 0, 255), true);
	
	InitializeStartIcon();
	SpawnRandomBackground();
	m_start = g_theAudio->CreateOrGetSound("Data/Audio/FirstStart.mp3");
	m_startPlayback = g_theAudio->StartSound(m_start, false, 0.1f);

	g_theEventSystem->SubscribeEventCallbackFunction("Keys", Game::Event_KeysAndFuncs);
	g_theEventSystem->SubscribeEventCallbackFunction("SetTimeScale", Game::Event_SetTimeScale);

	
	InitializePortData();
}

void Game::Update()
{	
	HandleInput();

	float deltaSeconds = m_clock->GetDeltaSeconds();

	if (!m_isAttractMode)
	{
		g_theAudio->StopSound(m_startPlayback);
		UpdateEntities(deltaSeconds);
		UpdateWave(deltaSeconds);
		CheckEnemiesVsShips();
		CheckBulletsVsEnemies();
		CheckShipVsShip(*m_playerShipA, *m_playerShipB);
		DeleteGarbages();
		UpdateMusic(deltaSeconds);
	} 
	else 
	{
		
		UpdateAttractMode(deltaSeconds);
	}

	UpdateACameras(deltaSeconds);

	if (m_multiplayer)
	{
		UpdateBCameras(deltaSeconds);
	}

	
}

void Game::Render() const
{
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
	
	if (!m_isAttractMode)
	{
		if (m_clock->IsPaused())
		{
			g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		}
		else
		{
			g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
		}
		RenderGame();
	}
	else
	{
		RenderAttractMode();
	}
	
	RenderDevConsole();
}

void Game::RenderDevConsole() const
{
	g_theRenderer->BeginCamera(m_screenCamera);

	if (g_theDevConsole)
	{
		g_theDevConsole->Render(AABB2(0, 0, 1600, 800));
	}

	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderGame() const
{
	if (!m_multiplayer)
	{
		g_theRenderer->SetViewport(m_fullport);
		g_theRenderer->BeginCamera(m_worldCameraA);
		RenderEntities();
		DebugRender();
		g_theRenderer->EndCamera(m_worldCameraA);
	}
	else
	{

		g_theRenderer->SetViewport(m_leftport);
		g_theRenderer->BeginCamera(m_worldCameraA);
		RenderEntities();
		DebugRender();
		g_theRenderer->EndCamera(m_worldCameraA);

		g_theRenderer->SetViewport(m_rightport);
		g_theRenderer->BeginCamera(m_worldCameraB);
		RenderEntities();
		DebugRender();
		g_theRenderer->EndCamera(m_worldCameraB);
	}

	g_theRenderer->SetViewport(m_fullport);
	g_theRenderer->BeginCamera(m_screenCamera);
	RenderUI();
	g_theRenderer->EndCamera(m_screenCamera);

}

void Game::RenderUI() const
{
	RenderHealth();
	RenderTutorialUI();
}


void Game::PlayMusic()
{
	m_music = g_theAudio->CreateOrGetSound("Data/Audio/BuMianZhiYe.mp3");
	m_musicPlayback = g_theAudio->StartSound(m_music, true, 0.01f);
}

void Game::UpdateACameras(float deltaSeconds)
{
	if (deltaSeconds == 0.f)
	{
		return;
	}

	m_screenCamera.SetOrthographicView(Vec2(0, 0), Vec2(1600, 800));

	float xOffset;
	float yOffset;
	if (!m_multiplayer)
	{
		xOffset = 120.f;
		yOffset = 60.f;
	}
	else
	{
		xOffset = 60.f;
		yOffset = 60.f;
	}

	m_worldCamShakeTraumaA -= CAM_SHAKE_REDUCTION_PER_SECOND * deltaSeconds;
	m_worldCamShakeTraumaA = GetClampedZeroToOne(m_worldCamShakeTraumaA);
	
	Vec2 worldCamMinsA(m_playerShipA->GetPosition().x - xOffset, m_playerShipA->GetPosition().y - yOffset);
	Vec2 worldCamMaxsA(m_playerShipA->GetPosition().x + xOffset, m_playerShipA->GetPosition().y + yOffset);

	if (worldCamMinsA.x < 0.f)
	{
		worldCamMinsA.x = 0.f;
		worldCamMaxsA.x = xOffset * 2;
	}
	else if (worldCamMaxsA.x > WORLD_SIZE_X)
	{
		worldCamMinsA.x = WORLD_SIZE_X - (xOffset * 2);
		worldCamMaxsA.x = WORLD_SIZE_X;
	}

	if (worldCamMinsA.y < 0.f)
	{
		worldCamMinsA.y = 0.f;
		worldCamMaxsA.y = yOffset * 2;
	}
	else if (worldCamMaxsA.y > WORLD_SIZE_Y)
	{
		worldCamMinsA.y = WORLD_SIZE_Y - (yOffset * 2);
		worldCamMaxsA.y = WORLD_SIZE_Y;
	}

	float camShakeAmountA = m_worldCamShakeTraumaA * CAM_SHAKE_MAX;
	float worldCamShakeXA = m_rng->RollRandomFloatInRange(-camShakeAmountA, camShakeAmountA);
	float worldCamShakeYA = m_rng->RollRandomFloatInRange(-camShakeAmountA, camShakeAmountA);
	Vec2 worldCamShakeA(worldCamShakeXA, worldCamShakeYA);
	worldCamMinsA += worldCamShakeA;
	worldCamMaxsA += worldCamShakeA;
	m_worldCameraA.SetOrthographicView(worldCamMinsA, worldCamMaxsA);


	
}

void Game::UpdateBCameras(float deltaSeconds)
{
	if (deltaSeconds == 0.f)
	{
		return;
	}

	float xOffset = 60.f;
	float yOffset = 60.f;


	m_worldCamShakeTraumaB -= CAM_SHAKE_REDUCTION_PER_SECOND * deltaSeconds;
	m_worldCamShakeTraumaB = GetClampedZeroToOne(m_worldCamShakeTraumaB);

	Vec2 worldCamMinsB(m_playerShipB->GetPosition().x - xOffset, m_playerShipB->GetPosition().y - yOffset);
	Vec2 worldCamMaxsB(m_playerShipB->GetPosition().x + xOffset, m_playerShipB->GetPosition().y + yOffset);

	if (worldCamMinsB.x < 0.f)
	{
		worldCamMinsB.x = 0.f;
		worldCamMaxsB.x = xOffset * 2;
	}
	else if (worldCamMaxsB.x > WORLD_SIZE_X)
	{
		worldCamMinsB.x = WORLD_SIZE_X - xOffset * 2;
		worldCamMaxsB.x = WORLD_SIZE_X;
	}

	if (worldCamMinsB.y < 0.f)
	{
		worldCamMinsB.y = 0.f;
		worldCamMaxsB.y = yOffset * 2;
	}
	else if (worldCamMaxsB.y > WORLD_SIZE_Y)
	{
		worldCamMinsB.y = WORLD_SIZE_Y - (yOffset * 2);
		worldCamMaxsB.y = WORLD_SIZE_Y;
	}

	float camShakeAmountB = m_worldCamShakeTraumaB * CAM_SHAKE_MAX;
	float worldCamShakeXB = m_rng->RollRandomFloatInRange(-camShakeAmountB, camShakeAmountB);
	float worldCamShakeYB = m_rng->RollRandomFloatInRange(-camShakeAmountB, camShakeAmountB);
	Vec2 worldCamShakeB(worldCamShakeXB, worldCamShakeYB);
	worldCamMinsB += worldCamShakeB;
	worldCamMaxsB += worldCamShakeB;


	m_worldCameraB.SetOrthographicView(worldCamMinsB, worldCamMaxsB);
}

void Game::UpdateMusic(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (!m_gameMusicStart)
	{
		m_gameMusicStart = true;
		PlayMusic();
	}

	if (m_muteMusic)
	{
		g_theAudio->SetSoundPlaybackVolume(m_musicPlayback, 0.0f);
	}
	else
	{
		g_theAudio->SetSoundPlaybackVolume(m_musicPlayback, 0.01f);
	}
}

void Game::AddCameraShakeTrauma(float shake, bool isPlayerB)
{
	if (!isPlayerB)
	{
		m_worldCamShakeTraumaA += shake;
	}
	else
	{
		m_worldCamShakeTraumaB += shake;
	}
}

Entity* Game::FindEnemyOverlappingDisc(Vec2 const& discCenter, float discRadius) const
{
	for (int asIndex = 0; asIndex < MAX_ASTEROIDS; ++asIndex)
	{
		Asteroid* asteroid = dynamic_cast<Asteroid*> (m_asteroids[asIndex]);
		if (IsAlive(asteroid) && DoDiscsOverlap(asteroid->GetPosition(), asteroid->GetPhysicsRadius(), 
											    discCenter, discRadius))
		{
			return asteroid;
		}
	}

	for (int beetIndex = 0; beetIndex < MAX_BETTLES; ++beetIndex)
	{
		Bettle* bettle = dynamic_cast<Bettle*> (m_beetles[beetIndex]);
		if (IsAlive(bettle) && DoDiscsOverlap(bettle->GetPosition(), bettle->GetPhysicsRadius(), 
											  discCenter, discRadius))
		{
			return bettle;
		}
	}

	for (int waspIndex = 0; waspIndex < MAX_WASPS; ++waspIndex)
	{
		Wasp* wasp = dynamic_cast<Wasp*> (m_wasps[waspIndex]);
		if (IsAlive(wasp) && DoDiscsOverlap(wasp->GetPosition(), wasp->GetPhysicsRadius(), 
											discCenter, discRadius))
		{
			return wasp;
		}
	}
	return nullptr;
}


	
bool Game::IsAlive(Entity* entity) const
{
	if (entity == nullptr)
	{
		return false;
	}
	if (!entity->IsAlive())
	{
		return false;
	}
	return true;
}

void Game::UpdateAttractMode(float deltaSeconds)
{

	if (!m_startAlphaUp && m_blinkPeriod < 0.5f)
	{
		m_blinkPeriod += deltaSeconds;
	}
	else if(m_startAlphaUp && m_blinkPeriod > 0.f)
	{
		m_blinkPeriod -= deltaSeconds;
	}
	
	if (GetClamped(m_blinkPeriod, 0.f, 0.5f) >= 0.5f || GetClamped(m_blinkPeriod, 0.f, 0.5f) <= 0.f)
	{
		m_startAlphaUp = !m_startAlphaUp;
	}

	if (!m_fakeShipMoveBack && m_movePeriod < 1.f)
	{
		m_movePeriod += deltaSeconds;
	}
	else if (m_fakeShipMoveBack && m_movePeriod > -1.f)
	{
		m_movePeriod -= deltaSeconds;
	}

	if (GetClamped(m_movePeriod, -1.f, 1.f) >= 1.f || GetClamped(m_movePeriod, -1.f, 1.f) <= -1.f)
	{
		m_fakeShipMoveBack = !m_fakeShipMoveBack;
	}
}

void Game::RenderAttractMode() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	RenderFakeShip(80.f, 0.f, Vec2(400.f + m_movePeriod * 40.f, 400.f), Rgba8(102, 153, 204, 255));

	if (m_multiplayer)
	{
		RenderFakeShip(80.f, 180.f, Vec2(1200.f - m_movePeriod * 40.f, 400.f), Rgba8(153, 0, 0, 255));
	}

	Vertex_PCU startSpaceVerts[NUM_WASP_VERTS];

	for (int vertIndex = 0; vertIndex < 3; ++vertIndex)
	{
		startSpaceVerts[vertIndex] = m_startIcon[vertIndex];
		float alpha = RangeMapClamped(m_blinkPeriod, 0.f, 0.5f, 100.f, 200.f);
		Rgba8 colorNow = m_startColor;
		colorNow.a = static_cast<unsigned char> (alpha);

		startSpaceVerts[vertIndex].m_color = colorNow;
	}
	TransformVertexArrayXY3D(NUM_WASP_VERTS, &startSpaceVerts[0], 50.f, 0.f, Vec2(800.f, 400.f));
	g_theRenderer->DrawVertexArray(NUM_WASP_VERTS, &startSpaceVerts[0]);
	
	std::vector<Vertex_PCU> textVerts;
	AddVertsForTextTriangles2D(textVerts, "Press [SPACE] to Start", Vec2(630.f, 80.f), 30.f, Rgba8(255,255,255, 
							   static_cast<unsigned char> (RangeMapClamped(m_blinkPeriod, 0.f, 0.5f, 200.f, 100.f))), .4f);
	if (!m_multiplayer)
	{
		AddVertsForTextTriangles2D(textVerts, "Press [M] for Multiplayer", Vec2(610.f, 150.f), 30.f, Rgba8(255, 255, 255, 
								   static_cast<unsigned char> (RangeMapClamped(m_blinkPeriod, 0.f, 0.5f, 200.f, 100.f))), .4f);
	}
	else
	{
		AddVertsForTextTriangles2D(textVerts, "Press [M] for Singleplayer", Vec2(610.f, 150.f), 30.f, Rgba8(255, 255, 255, 
								   static_cast<unsigned char> (RangeMapClamped(m_blinkPeriod, 0.f, 0.5f, 200.f, 100.f))), .4f);
	}
	g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::Shutdown()
{
	g_theAudio->StopSound(m_musicPlayback);
}

void Game::DebugRender() const
{
	if (m_isDebugActive)
	{
		for (int bulletIndex = 0; bulletIndex < MAX_BULLETS; ++bulletIndex)
		{
			Bullet const* bullet = dynamic_cast<Bullet*>(m_bullets[bulletIndex]);
			if (bullet)
			{
				bullet->DebugRender();
				


				DebugDrawLine(bullet->GetPosition(), m_playerShipA->GetPosition(), DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));
				
				if (m_multiplayer)
				{
					DebugDrawLine(bullet->GetPosition(), m_playerShipB->GetPosition(), DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));

				}
			}
		}

		for (int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; ++asteroidIndex)
		{
			Asteroid const* asteroid = dynamic_cast<Asteroid*>(m_asteroids[asteroidIndex]);
			if (asteroid)
			{
				asteroid->DebugRender();
				
				DebugDrawLine(asteroid->GetPosition(), m_playerShipA->GetPosition(), DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));
				if (m_multiplayer)
				{
					DebugDrawLine(asteroid->GetPosition(), m_playerShipB->GetPosition(), DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));
				}
			}
		}

		for (int beetIndex = 0; beetIndex < MAX_BETTLES; ++beetIndex)
		{
			Bettle const* bettle = dynamic_cast<Bettle*>(m_beetles[beetIndex]);
			if (bettle)
			{

				bettle->DebugRender();
				

				DebugDrawLine(bettle->GetPosition(), m_playerShipA->GetPosition(), DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));
				if (m_multiplayer)
				{
					DebugDrawLine(bettle->GetPosition(), m_playerShipB->GetPosition(), DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));
				}

			}
		}

		for (int waspIndex = 0; waspIndex < MAX_WASPS; ++waspIndex)
		{
			Wasp const* wasp = dynamic_cast<Wasp*>(m_wasps[waspIndex]);
			if (wasp)
			{
				wasp->DebugRender();
				

				DebugDrawLine(wasp->GetPosition(), m_playerShipA->GetPosition(), DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));
				if (m_multiplayer)
				{
					DebugDrawLine(wasp->GetPosition(), m_playerShipB->GetPosition(), DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50, 255));
				}

			}
		}
		m_playerShipA->DebugRender();
		if (m_multiplayer)
		{
			m_playerShipB->DebugRender();
		}
	}

}

void Game::HandleInput()
{
	if (g_theInput->WasKeyJustPressed('I'))
	{
		SpawnRandomAsteroid();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebugActive = !m_isDebugActive;
	}

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_clock->StepSingleFrame();
	}

	if (g_theInput->WasKeyJustPressed('P') || g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_LB))
	{
		m_clock->TogglePause();
	}

	if (g_theInput->IsKeyDown('T') || g_theInput->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_LS))
	{
		m_clock->SetTimeScale(0.1f);
	}
	else
	{
		m_clock->SetTimeScale(1.f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_BACK))
	{
		if (m_isAttractMode)
		{
			g_theApp->HandleQuitRequested();
		}
		else
		{
			g_theApp->ResetGame();
			SoundID back = g_theAudio->CreateOrGetSound("Data/Audio/Back.wav");
			g_theAudio->StartSound(back, false, 0.1f);
		}

	}

	if (m_isAttractMode)
	{
		if (g_theInput->WasKeyJustPressed('M'))
		{
			m_multiplayer = !m_multiplayer;
			SoundID multiplayer = g_theAudio->CreateOrGetSound("Data/Audio/Multiplayer.wav");
			g_theAudio->StartSound(multiplayer, false, 0.1f);
		}

		if (g_theInput->WasKeyJustPressed(' ') ||
			g_theInput->WasKeyJustPressed('N') ||
			g_theInput->GetController(0).WasButtonJustPressed(XBOX_BUTTON_A) ||
			g_theInput->GetController(0).WasButtonJustPressed(XBOX_BUTTON_START))
		{
			m_isAttractMode = false;
			FireEvent("Keys");
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		g_theApp->ResetGame();
	}

	if (g_theInput->WasKeyJustPressed('Q'))
	{
		m_muteMusic = !m_muteMusic;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		m_isConsoleOpen = !m_isConsoleOpen;
		g_theDevConsole->ToggleOpen();
	}

}

void Game::SpawnRandomAsteroid()
{
	float randomX = 0.f;
	float randomY = 0.f;
	int edge = m_rng->RollRandomIntInRange(0, 3);

	switch (edge)
	{
	case 0:
		randomX = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_X);
		randomY = WORLD_SIZE_Y + ASTEROID_COSMETIC_RADIUS;
		break;
	case 1:
		randomX = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_X);
		randomY = -ASTEROID_COSMETIC_RADIUS;
		break;
	case 2:
		randomX = -ASTEROID_COSMETIC_RADIUS;
		randomY = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_Y);
		break;
	case 3:
		randomX = WORLD_SIZE_X + ASTEROID_COSMETIC_RADIUS;
		randomY = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_Y);
		break;
	}

	float randomOrientationDeg = m_rng->RollRandomFloatInRange(0.f, 360.f);
	Asteroid* m_asteroid = new Asteroid(this, Vec2(randomX, randomY), randomOrientationDeg, Rgba8(100, 100, 100, 255));
	for (int i = 0; i < MAX_ASTEROIDS; ++i)
	{
		if (m_asteroids[i] == nullptr)
		{
			m_asteroids[i] = m_asteroid;
			return;
		}
	}
	ERROR_RECOVERABLE("Cannot spawn new Asteroid; all slots are full.");
}

void Game::SpawnRandomBettle()
{
	float randomX = 0.f;
	float randomY = 0.f;
	int edge = m_rng->RollRandomIntInRange(0, 3);

	switch (edge)
	{
	case 0:
		randomX = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_X);
		randomY = WORLD_SIZE_Y + BEETLE_COSMETIC_RADIUS;
		break;
	case 1:
		randomX = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_X);
		randomY = -BEETLE_COSMETIC_RADIUS;
		break;
	case 2:
		randomX = -BEETLE_COSMETIC_RADIUS;
		randomY = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_Y);
		break;
	case 3:
		randomX = WORLD_SIZE_X + BEETLE_COSMETIC_RADIUS;
		randomY = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_Y);
		break;
	}

	Bettle* bettle = new Bettle(this, Vec2(randomX, randomY), 0.f, Rgba8(0, 100, 50, 255));

	for (int i = 0; i < MAX_BETTLES; ++i)
	{
		if (m_beetles[i] == nullptr)
		{
			m_beetles[i] = bettle;
			return;
		}
	}
	ERROR_RECOVERABLE("Cannot spawn new Bettle; all slots are full.");
}

void Game::SpawnRandomWasp()
{
	float randomX = 0.f;
	float randomY = 0.f;
	int edge = m_rng->RollRandomIntInRange(0, 3);

	switch (edge)
	{
	case 0:
		randomX = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_X);
		randomY = WORLD_SIZE_Y + WASP_COSMETIC_RADIUS;
		break;
	case 1:
		randomX = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_X);
		randomY = -WASP_COSMETIC_RADIUS;
		break;
	case 2:
		randomX = -WASP_COSMETIC_RADIUS;
		randomY = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_Y);
		break;
	case 3:
		randomX = WORLD_SIZE_X + WASP_COSMETIC_RADIUS;
		randomY = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_Y);
		break;
	}

	Wasp* wasp = new Wasp(this, Vec2(randomX, randomY), 0.f, Rgba8(255, 255, 0, 255));
	for (int i = 0; i < MAX_WASPS; ++i)
	{
		if (m_wasps[i] == nullptr)
		{
			m_wasps[i] = wasp;
			return;
		}
	}
	ERROR_RECOVERABLE("Cannot spawn new Wasp; all slots are full.");
}

void Game::SpawnBullet(Vec2 const& position, float orientationDegrees, Vec2 velocity)
{
	Bullet* m_bullet = new Bullet(this, position, orientationDegrees, Rgba8(255, 255, 0, 255), velocity);
	for (int i = 0; i < MAX_BULLETS; ++i)
	{
		if (m_bullets[i] == nullptr)
		{
			m_bullets[i] = m_bullet;
			return;  
		}
	}
	ERROR_RECOVERABLE("Cannot spawn new Bullet; all slots are full.");
}

void Game::SpawnBullets(Vec2 const& position, float orientationDegrees, Vec2 velocity, int numberOfBullets, float spreadAngle)
{
	float angleIncrement = spreadAngle / static_cast<float>(numberOfBullets - 1);

	for (int i = 0; i < numberOfBullets; ++i)
	{
		float bulletAngle = orientationDegrees - (spreadAngle / 2.0f) + i * angleIncrement;

		Vec2 bulletVelocity = velocity + Vec2(CosDegrees(bulletAngle), SinDegrees(bulletAngle)) * BULLET_SPEED;

		SpawnBullet(position, bulletAngle, bulletVelocity);
	}
}

void Game::SpawnNewDebris(Vec2 const& position, Vec2 const& velocity, float radius, Rgba8 const& color)
{
	Debris* m_deb = new Debris(this, position, velocity, radius, color);
	for (int i = 0; i < MAX_DEBRIS; ++i)
	{
		if (m_debris[i] == nullptr)
		{
			m_debris[i] = m_deb;
			return;
		}
	}
	ERROR_RECOVERABLE("Cannot spawn new Debris; all slots are full.");
	
}

void Game::SpawnNewDebrisCluster(int numDebris, Vec2 const& position, Vec2 const& averageVelocity, float spraySpeed, float radius, Rgba8 const& color)
{
	for (int i = 0; i < numDebris; ++i)
	{
		float thetaDegrees = m_rng->RollRandomFloatInRange(0.f, 360.f);
		float speed = m_rng->RollRandomFloatInRange(1.f, spraySpeed);
		Vec2 scatterVelocity = Vec2::MakeFromPolarDegrees(thetaDegrees, speed);
		Vec2 velocity = averageVelocity + scatterVelocity;
		SpawnNewDebris(position, velocity, radius, color);
	}
}

PlayerShip* Game::GetPlayership(int shipIndex) const
{
	if (shipIndex == 0)
	{
		return m_playerShipA;
	}
	else
	{
		return m_playerShipB;
	}
}

bool Game::Event_KeysAndFuncs(EventArgs& args)
{
	UNUSED(args);
	Rgba8 gameColor = Rgba8::PINK;
	g_theDevConsole->AddLine(gameColor, "Keys:");
	g_theDevConsole->AddLine(gameColor, "	[S/F]    - Rotate the Ship: ");
	g_theDevConsole->AddLine(gameColor, "	[E]      - Accelerate");
	g_theDevConsole->AddLine(gameColor, "	[J]      - Fire");
	g_theDevConsole->AddLine(gameColor, "	[K/L]    - Special Fire");
	g_theDevConsole->AddLine(gameColor, "	[SPACE]  - Invisible");
	g_theDevConsole->AddLine(gameColor, "	[N]      - Respawn");
	g_theDevConsole->AddLine(gameColor, "	[P]      - Pause/Unpause");
	g_theDevConsole->AddLine(gameColor, "	[O]      - Step Single Frame");
	g_theDevConsole->AddLine(gameColor, "	[T]	     - Toggle Slow Mode");
	g_theDevConsole->AddLine(gameColor, "	[Q]	     - Mute/Unmute Music");
	g_theDevConsole->AddLine(gameColor, "	[ESC]    - Back");
	g_theDevConsole->AddLine(gameColor, "	[I]	     - Spawn 1 Asteroid");
	g_theDevConsole->AddLine(gameColor, "	[F1]     - Debug Draw");
	g_theDevConsole->AddLine(gameColor, "	[F8]     - Reset Game");
	
	return true;
}

bool Game::Event_SetTimeScale(EventArgs& args)
{
	float timeScale = args.GetValue("scale", -1.f);

	if (timeScale < 0.1f || timeScale > 1.0f)
	{
		g_theDevConsole->AddLine(DevConsole::ERROR_COLOR, "Error: Scale Missing or Incorrect. Must be 0.1 to 1.0!");
		g_theDevConsole->AddLine(DevConsole::WARNING, "Usage: SetTimeScale scale=1.0");
		return false;
	}

	g_theApp->m_game->m_clock->SetTimeScale(timeScale);
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, Stringf("Time scale set to: %.1f", timeScale));
	return true;
}



void Game::RenderHealth() const
{
	float interval = 50.0f;

	for (int healthAIndex = 0; healthAIndex < m_playerShipA->GetExtraLives(); ++healthAIndex)
	{
		RenderFakeShip(10.0f, 90.f, Vec2(interval * (healthAIndex + 1), 750.f), Rgba8(102, 153, 204, 255));
	}

	if (m_multiplayer)
	{
		for (int healthBIndex = m_playerShipB->GetExtraLives(); healthBIndex > 0; --healthBIndex)
		{
			RenderFakeShip(10.0f, 90.f, Vec2(800 + interval * (healthBIndex), 750.f), Rgba8(153, 0, 0, 255));
		}
	}
}

void Game::RenderTutorialUI() const
{
	std::vector<Vertex_PCU> textVerts;
	AddVertsForTextTriangles2D(textVerts, "[J]					 -> Fire", 
							   Vec2(35.f, 700.f), 15, Rgba8(255, 255, 255, 200));
	AddVertsForTextTriangles2D(textVerts, "[K]					 -> Bullet Burst Alpha", 
							   Vec2(35.f, 680.f), 15, Rgba8(255, 255, 255, 200));
	AddVertsForTextTriangles2D(textVerts, "[L]					 -> Bullet Burst Beta", 
							   Vec2(35.f, 660.f), 15, Rgba8(255, 255, 255, 200));
	AddVertsForTextTriangles2D(textVerts, "[SPACE]		-> Stealthy", 
							   Vec2(35.f, 640.f), 15, Rgba8(255, 255, 255, 200));
	
	AddVertsForTextTriangles2D(textVerts, "[ESC] -> Back to Menu",
		Vec2(1340, 750), 15, Rgba8(255, 255, 255, 200));

	if (Clock::GetSystemClock().IsPaused())
	{
		AddVertsForTextTriangles2D(textVerts, "[P]			-> Unpause", 
								   Vec2(1340, 730), 15, Rgba8(255, 255, 255, 200));
	}
	else
	{
		AddVertsForTextTriangles2D(textVerts, "[P]			-> Pause", 
								   Vec2(1340, 730), 15, Rgba8(255, 255, 255, 200));
	}

	if (Clock::GetSystemClock().IsPaused())
	{
		AddVertsForTextTriangles2D(textVerts, "[Q]			-> Unmute Music",
			Vec2(1340, 710), 15, Rgba8(255, 255, 255, 200));
	}
	else
	{
		AddVertsForTextTriangles2D(textVerts, "[Q]			-> Mute Music",
			Vec2(1340, 710), 15, Rgba8(255, 255, 255, 200));
	}
	

	

	if (!m_playerShipA->IsAlive())
	{
		if (!m_multiplayer)
		{
			if (m_playerShipA->GetExtraLives() > 0)
			{
				AddVertsForTextTriangles2D(textVerts, "Press [N] to Respwan", 
										   Vec2(450, 375), 50, Rgba8(255, 255, 255, 200));
			}
			else
			{
				AddVertsForTextTriangles2D(textVerts, "Game Over", 
										   Vec2(500, 350), 100, Rgba8(255, 0, 0, 200));
			}
		}
		else
		{
			if (m_playerShipA->GetExtraLives() > 0)
			{
				AddVertsForTextTriangles2D(textVerts, "Press [N] to Respwan", 
										   Vec2(200, 385), 30, Rgba8(255, 255, 255, 200));
			}
		}
	}

	if (m_multiplayer)
	{

		DebugDrawLine(Vec2(800, 800), Vec2(800, 0), 2.f, Rgba8(255, 255, 255, 100));

		AddVertsForTextTriangles2D(textVerts, "[A]		-> Fire", 
								   Vec2(835.f, 700.f), 15, Rgba8(255, 255, 255, 200));
		AddVertsForTextTriangles2D(textVerts, "[B]		-> Bullet Burst Alpha", 
								   Vec2(835.f, 680.f), 15, Rgba8(255, 255, 255, 200));
		AddVertsForTextTriangles2D(textVerts, "[X]		-> Bullet Burst Beta", 
								   Vec2(835.f, 660.f), 15, Rgba8(255, 255, 255, 200));
		AddVertsForTextTriangles2D(textVerts, "[Y]		-> Stealthy", 
								   Vec2(835.f, 640.f), 15, Rgba8(255, 255, 255, 200));
		if (!m_playerShipB->IsAlive() && m_playerShipB->GetExtraLives() > 0)
		{
			AddVertsForTextTriangles2D(textVerts, "Press [START] to Respwan", 
								       Vec2(950.f, 395), 30, Rgba8(255, 255, 255, 200));
		}
	}

	if (m_win)
	{
		AddVertsForTextTriangles2D(textVerts, "You Win", Vec2(570, 350), 100, Rgba8(0, 255, 0, 200));
	}

	if (m_lose)
	{
		AddVertsForTextTriangles2D(textVerts, "Game Over", Vec2(500, 350), 100, Rgba8(255, 0, 0, 200));
	}



	g_theRenderer->DrawVertexArray(static_cast<int> (textVerts.size()), textVerts.data());
}


void Game::RenderFakeShip(float scale, float rotationDegrees, Vec2 translation, Rgba8 color) const
{
	Vertex_PCU translucentFakeShip[NUM_SHIP_VERTS];

	PlayerShip::InitializeVerts(&translucentFakeShip[0], color);

	TransformVertexArrayXY3D(NUM_SHIP_VERTS, &translucentFakeShip[0], scale, rotationDegrees, translation);

	g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, &translucentFakeShip[0]);
}

void Game::SpawnNewWave()
{
	int numBeetles = m_currentWave * 8;
	int numWasps = m_currentWave * 4;
	int numAsteroids = m_currentWave * 10 + 20;

	if (m_multiplayer)
	{
		numBeetles *= 2;
		numWasps *= 2;
		numAsteroids *= 2;
	}

	for (int betIndex = 0; betIndex < numBeetles; ++betIndex)
	{
		SpawnRandomBettle();
	}
	for (int waspIndex = 0; waspIndex < numWasps; ++waspIndex)
	{
		SpawnRandomWasp();
	}
	for (int asIndex = 0; asIndex < numAsteroids; ++asIndex)
	{
		SpawnRandomAsteroid();
	}


}

void Game::SpawnRandomBackground()
{
	
	float blinkPeriod = 1.f;
	Timer* blinkTimer = new Timer(blinkPeriod, m_clock);


	for (int i = 0; i < MAX_STARS; ++i)
	{
		float randomX = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_X);
		float randomY = m_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_Y);

		Star* star = new Star(this, Vec2(randomX, randomY), 0.f, Rgba8(255, 255, 255, 255));
		star->m_blinkTimer = blinkTimer;
		m_stars[i] = star;

	}
}

void Game::UpdateWave(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	m_win = (m_currentWave > m_maxWaves) && m_waveComplete;
	if (!m_multiplayer)
	{
		m_lose = (!m_playerShipA->IsAlive()) && (m_playerShipA->GetExtraLives() == 0);
	}
	else
	{
		m_lose = ((!m_playerShipA->IsAlive()) && (m_playerShipA->GetExtraLives() == 0)) &&
			   ((!m_playerShipB->IsAlive()) && (m_playerShipB->GetExtraLives() == 0));
	}

	HandleGameWinOrLose();
	HandleGameOver();
	HandleWaveComplete();
	CheckWaveEnd();
}

bool Game::DoEntitiesOverlap(Entity const& a, Entity const& b)
{
	Vec2 const& posA = a.GetPosition();
	Vec2 const& posB = b.GetPosition();
	float const& phyRadA = a.GetPhysicsRadius();
	float const& phyRadB = b.GetPhysicsRadius();
	return DoDiscsOverlap(posA,phyRadA, posB, phyRadB);
}

void Game::RenderEntities() const
{
	RenderEntityList(MAX_STARS, m_stars);
	RenderEntityList(MAX_BULLETS, m_bullets);
	RenderEntityList(MAX_ASTEROIDS, m_asteroids);
	RenderEntityList(MAX_DEBRIS, m_debris);
	RenderEntityList(MAX_BETTLES, m_beetles);
	RenderEntityList(MAX_WASPS, m_wasps);

	RenderShip(m_playerShipA);
	if (m_multiplayer)
	{
		RenderShip(m_playerShipB);
	}
	
}

void Game::RenderEntityList(int listMaxSize, Entity* const list[]) const
{
	for (int entityIndex = 0; entityIndex < listMaxSize; ++entityIndex)
	{
		Entity* entity = list[entityIndex];
		if (IsAlive(entity))
		{
			entity->Render();
		}
	}
}

void Game::RenderShip(PlayerShip* ship) const
{
	if (ship->IsAlive())
	{
		ship->Render();
	}
	if (ship->m_isInvisible)
	{
		Vec2 playerPos = ship->GetPosition();
		RenderFakeShip(2.f, ship->GetOrientionDegrees(), Vec2(playerPos.x + 4.f, playerPos.y), Rgba8(192, 192, 192, 127));
		RenderFakeShip(2.f, ship->GetOrientionDegrees(), Vec2(playerPos.x - 4.f, playerPos.y), Rgba8(192, 192, 192, 127));
		RenderFakeShip(2.f, ship->GetOrientionDegrees(), Vec2(playerPos.x, playerPos.y + 4.f), Rgba8(192, 192, 192, 127));
		RenderFakeShip(2.f, ship->GetOrientionDegrees(), Vec2(playerPos.x, playerPos.y - 4.f), Rgba8(192, 192, 192, 127));
	}
}

void Game::HandleWaveComplete()
{
	if (m_waveComplete && m_currentWave <= m_maxWaves && !m_gameOver)
	{
		SpawnNewWave();
		m_waveComplete = false;
		m_currentWave += 1;
		if (m_currentWave <= m_maxWaves)
		{
			SoundID newWave = g_theAudio->CreateOrGetSound("Data/Audio/NewWave.wav");
			g_theAudio->StartSound(newWave, false, 0.3f);
		}
	}
}

void Game::HandleGameOver()
{
	float deltaSeconds = m_clock->GetDeltaSeconds();

	if (m_gameOver)
	{
		if (m_resetTimer >= 3.f)
		{
			m_isAttractMode = true;
			g_theApp->ResetGame();
			
		}
		else
		{
			m_resetTimer += deltaSeconds;
		}
	}
}

void Game::HandleGameWinOrLose()
{
	if (!m_gameOver && (m_win || m_lose))
	{
		m_gameOver = true;

		if (m_win)
		{
			SoundID winSound = g_theAudio->CreateOrGetSound("Data/Audio/win.mp3");
			g_theAudio->StartSound(winSound, false, 0.5f);
		}
		else if (m_lose)
		{
			SoundID loseSound = g_theAudio->CreateOrGetSound("Data/Audio/lose.wav");
			g_theAudio->StartSound(loseSound, false, 0.5f);
		}
	}
}

void Game::CheckWaveEnd()
{
	for (int betIndex = 0; betIndex < MAX_BETTLES; ++betIndex)
	{
		if (m_beetles[betIndex] != nullptr)
		{
			return;
		}
	}

	for (int waspIndex = 0; waspIndex < MAX_WASPS; ++waspIndex)
	{
		if (m_wasps[waspIndex] != nullptr)
		{
			return;
		}
	}
	m_waveComplete = true;
}

void Game::DeleteGarbages()
{
	DeleteGarbageList(MAX_BULLETS, m_bullets);
	DeleteGarbageList(MAX_ASTEROIDS, m_asteroids);
	DeleteGarbageList(MAX_BETTLES, m_beetles);
	DeleteGarbageList(MAX_DEBRIS, m_debris);
	DeleteGarbageList(MAX_WASPS, m_wasps);
}

void Game::DeleteGarbageList(int listMaxSize, Entity* list[])
{
	for (int entityIndex = 0; entityIndex < listMaxSize; ++entityIndex)
	{
		Entity* entity = list[entityIndex];
		if (entity && entity->GetIsGarbage())
		{
			delete entity;
			list[entityIndex] = nullptr;
		}
	}
}

void Game::InitializeStartIcon()
{
	m_startIcon[0].m_position = Vec3(-2.f, 2.f, 0.f);
	m_startIcon[1].m_position = Vec3(2.f, 0.f, 0.f);
	m_startIcon[2].m_position = Vec3(-2.f, -2.f, 0.f);
}

void Game::InitializePortData()
{
	float maxX = (float)g_theWindow->GetClientDimensions().x;
	float maxY = (float)g_theWindow->GetClientDimensions().y;


	m_fullport.TopLeftX = 0.f;
	m_fullport.TopLeftY = 0.f;
	m_fullport.Width = maxX;
	m_fullport.Height = maxY;


	m_leftport.TopLeftX = 0.f;
	m_leftport.TopLeftY = 0.f;
	m_leftport.Width = maxX * 0.5f;
	m_leftport.Height = maxY;


	m_rightport.TopLeftX = maxX * 0.5f;
	m_rightport.TopLeftY = 0.f;
	m_rightport.Width = maxX * 0.5f;
	m_rightport.Height = maxY;

}

void Game::UpdateEntities(float deltaSeconds)
{
	m_playerShipA->Update(deltaSeconds);
	if (m_multiplayer)
	{
		m_playerShipB->Update(deltaSeconds);
	}
	UpdateEntityList(MAX_STARS, m_stars, false, deltaSeconds);
	UpdateEntityList(MAX_BULLETS, m_bullets, false, deltaSeconds);
	UpdateEntityList(MAX_ASTEROIDS, m_asteroids, true, deltaSeconds);
	UpdateEntityList(MAX_BETTLES, m_beetles, true, deltaSeconds);
	UpdateEntityList(MAX_WASPS, m_wasps, true, deltaSeconds);
	UpdateEntityList(MAX_DEBRIS, m_debris, false, deltaSeconds);

}

void Game::UpdateEntityList(int listMaxSize, Entity* list[], bool checkOverlap, float deltaSeconds)
{
	for (int entityIndex = 0; entityIndex < listMaxSize; ++entityIndex)
	{
		Entity* entity = list[entityIndex];
		
		if (IsAlive(entity))
		{
			entity->Update(deltaSeconds);

			if (checkOverlap)
			{
				Entity* other = FindEnemyOverlappingDisc(entity->GetPosition(), entity->GetPhysicsRadius());
				if (other != nullptr && entity != other)
				{
					entity->PushOutOfEntity(other);
				}
			}
			
			
		}
	}

}

void Game::CheckBulletsVsEnemies()
{
	for (int buIndex = 0; buIndex < MAX_BULLETS; ++buIndex)
	{
		Bullet* bullet = dynamic_cast<Bullet*> (m_bullets[buIndex]);
		CheckBulletVsEnemyList(bullet, MAX_ASTEROIDS, m_asteroids);
		CheckBulletVsEnemyList(bullet, MAX_BETTLES, m_beetles);
		CheckBulletVsEnemyList(bullet, MAX_WASPS, m_wasps);
	}

}

void Game::CheckBulletVsEnemyList(Bullet* bullet, int listMaxSize, Entity* list[])
{
	for (int entityIndex = 0; entityIndex < listMaxSize; ++entityIndex)
	{
		Entity* entity = list[entityIndex];
		if (IsAlive(bullet) && IsAlive(entity))
		{
			CheckBulletVsEnemy(*bullet, *entity);
		}
	}
}

void Game::CheckBulletVsEnemy(Bullet& bullet, Entity& entity)
{
	if (DoEntitiesOverlap(bullet, entity))
	{
		bullet.Die();
		entity.BeHitted();
		if (entity.GetHealth() <= 0)
		{
			entity.Die();
		}
		
	}
}

void Game::CheckEnemiesVsShips()
{
	if (m_playerShipA != nullptr)
	{
		CheckEnemyListVsShips(MAX_ASTEROIDS, m_asteroids);
		CheckEnemyListVsShips(MAX_BETTLES, m_beetles);
		CheckEnemyListVsShips(MAX_WASPS, m_wasps);
	}
	
}

void Game::CheckEnemyListVsShips(int listSize, Entity* list[])
{
	for (int entityIndex = 0; entityIndex < listSize; ++entityIndex)
	{
		Entity* entity = list[entityIndex];
		if (entity)
		{
			if (m_playerShipA != nullptr)
			{
				if (!m_playerShipA->m_isInvisible)
				{
					CheckEnemyVsShip(*entity, *m_playerShipA);
				}
			}

			if (m_multiplayer)
			{
				if (m_playerShipB != nullptr)
				{
					if (!m_playerShipB->m_isInvisible) 
					{
						CheckEnemyVsShip(*entity, *m_playerShipB);
					}
				}
			}
		}
	}
}

void Game::CheckEnemyVsShip(Entity& entity, PlayerShip& ship)
{
	if (DoEntitiesOverlap(entity, ship))
	{
		if (ship.IsAlive())
		{
			ship.Die();
			entity.BeHitted();
			if (entity.GetHealth() <= 0)
			{
				entity.Die();
			}
		}
		
	}
}

void Game::CheckShipVsShip(PlayerShip& shipA, PlayerShip& shipB)
{
	if (DoEntitiesOverlap(shipA, shipB) && m_multiplayer)
	{
		if (shipA.IsAlive() && shipB.IsAlive())
		{
			shipA.ShipsCollision();
			shipB.ShipsCollision();
		}
	}
}
