#pragma once
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Clock.hpp"


class App;
class PlayerShip;
class Asteroid;
class Bullet;
class Debris;
class Bettle;
class Wasp;
class Star;
class Entity;



class Game 
{
public:

	Game(App* owner);	
	~Game();
	void Startup();

	void Update();
	void AddCameraShakeTrauma(float shake, bool isPlayerA);
	
	void Render() const;
	
	void PlayMusic();
	void Shutdown();
	void DebugRender() const;

	void HandleInput();
	
	void SpawnRandomAsteroid();
	void SpawnRandomBettle();
	void SpawnRandomWasp();
	void SpawnBullet(Vec2 const& position, float orientationDegrees, Vec2 velocity);
	void SpawnBullets(Vec2 const& position, float orientationDegrees, Vec2 velocity,int numberOfBullets, float spreadAngle);

	void SpawnNewDebris(Vec2 const& position, Vec2 const& velocity, float radius, Rgba8 const& color);
	void SpawnNewDebrisCluster(int numDebris, Vec2 const& position, Vec2 const& averageVelocity, float spraySpeed, float radius, Rgba8 const& color);

	PlayerShip* GetPlayership(int shipIndex) const;
	static bool Event_KeysAndFuncs(EventArgs& args);
	static bool Event_SetTimeScale(EventArgs& args);

public:
	App* m_App = nullptr;
	PlayerShip* m_playerShipA = nullptr;
	PlayerShip* m_playerShipB = nullptr;
	Entity* m_bullets[MAX_BULLETS] = {};
	Entity* m_asteroids[MAX_ASTEROIDS] = {};
	Entity* m_debris[MAX_DEBRIS] = {};
	Entity* m_beetles[MAX_BETTLES] = {};
	Entity* m_wasps[MAX_WASPS] = {};
	Entity* m_stars[MAX_STARS] = {};
	RandomNumberGenerator* m_rng = nullptr;
	Vertex_PCU m_startIcon[3];
	bool m_isDebugActive = false;
	bool m_isAttractMode = true;
	bool m_startAlphaUp = false;
	float m_blinkPeriod = 0.5f;
	bool m_fakeShipMoveBack = false;
	float m_movePeriod = 0.f;
	float m_resetTimer = 0.f;
	Rgba8 m_startColor = Rgba8(0, 255, 0, 255);
	int m_currentWave = 1;
	const int m_maxWaves = 5;
	bool m_waveComplete = true;
	Camera m_screenCamera;
	Camera m_worldCameraA;
	Camera m_worldCameraB;
	float m_worldCamShakeTraumaA;
	float m_worldCamShakeTraumaB;
	bool m_gameOver;
	bool m_gameMusicStart = false;
	SoundPlaybackID m_musicPlayback;
	SoundID m_music;
	SoundPlaybackID m_startPlayback;
	SoundID m_start;
	bool m_win;
	bool m_lose;
	bool m_multiplayer = false;
	bool m_muteMusic = false;
	bool m_isConsoleOpen = false;
	Clock* m_clock = nullptr;
	ViewportData m_fullport;
	ViewportData m_leftport;
	ViewportData m_rightport;

private:

	void InitializeStartIcon();

	void InitializePortData();
	void UpdateEntities(float deltaSeconds);
	void UpdateEntityList(int listMaxSize, Entity* list[], bool checkOverlap, float deltaSeconds);
	void UpdateAttractMode(float deltaSeconds);
	void UpdateWave(float deltaSeconds);
	void UpdateACameras(float deltaSeconds);
	void UpdateBCameras(float deltaSeconds);
	void UpdateMusic(float deltaSeconds);


	void RenderAttractMode() const;
	void RenderUI() const;
	void RenderHealth() const;
	void RenderTutorialUI() const;
	void RenderFakeShip(float scale, float rotationDegrees, Vec2 translation, Rgba8 color) const;
	void RenderGame() const;
	void RenderDevConsole() const;
	void RenderEntities() const;
	void RenderEntityList(int listMaxSize, Entity* const list[]) const;
	void RenderShip(PlayerShip* ship) const;

	void SpawnNewWave();
	void SpawnRandomBackground();
	void HandleWaveComplete();
	void HandleGameOver();
	void HandleGameWinOrLose();
	void CheckWaveEnd();


	void CheckBulletsVsEnemies();
	void CheckBulletVsEnemyList(Bullet* bullet, int listMaxSize, Entity* list[]);
	void CheckBulletVsEnemy(Bullet& bullet, Entity& entity);
	void CheckEnemiesVsShips();
	void CheckEnemyListVsShips(int listMaxSize, Entity* list[]);
	void CheckEnemyVsShip(Entity& entity, PlayerShip& ship);
	void CheckShipVsShip(PlayerShip& shipA, PlayerShip& shipB);
	bool DoEntitiesOverlap(Entity const& a, Entity const& b);

	Entity* FindEnemyOverlappingDisc(Vec2 const& discCneter, float discRadius) const;
	bool IsAlive(Entity* entity) const;


	void DeleteGarbages();
	void DeleteGarbageList(int listMaxSize, Entity* list[]);
	
	
};