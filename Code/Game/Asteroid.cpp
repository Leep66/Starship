#include "Game/Asteroid.hpp"
#include "Game/App.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include <math.h>

extern App* g_theApp;
extern Renderer* g_theRenderer;
extern AudioSystem* g_theAudio;
RandomNumberGenerator rng;

Asteroid::Asteroid(Game* owner, const Vec2& startPos, float orientationDeg, Rgba8 color)
	: Entity(owner, startPos, orientationDeg, color)
{
	m_rotateDegree = 0.f;
	m_angularVeclocity = rng.RollRandomFloatInRange(-200.f, 200.f);
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS;
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;
	m_health = 4;
	
	InitializeLocalVerts();
}

Asteroid::~Asteroid()
{

}

void Asteroid::Update(float deltaSeconds)
{
	float orientationRadians = ConvertDegreesToRadians(m_orientationDegrees);

	m_velocity = Vec2(ASTEROID_SPEED * cosf(orientationRadians), ASTEROID_SPEED * sinf(orientationRadians));

	m_position += (m_velocity * deltaSeconds);
	
	m_rotateDegree += (m_angularVeclocity * deltaSeconds);

	HandleBeHitted(deltaSeconds);
	HandleOffscreen();
	
}

void Asteroid::HandleBeHitted(float deltaSeconds)
{
	if (m_isHitted)
	{
		m_hittedTimer += deltaSeconds;
		if (m_hittedTimer >= m_hitColorDuration)
		{
			m_isHitted = false;
			m_hittedTimer = 0.0f;
			m_color = m_originalColor;
		}
	}
}

void Asteroid::HandleOffscreen()
{
	if (IsOffscreen())
	{
		if (m_position.x < 0.f) {
			m_position.x = WORLD_SIZE_X;
		}
		else if (m_position.x > WORLD_SIZE_X) {
			m_position.x = 0.f;
		}

		if (m_position.y < 0.f) {
			m_position.y = WORLD_SIZE_Y;
		}
		else if (m_position.y > WORLD_SIZE_Y) {
			m_position.y = 0.f;
		}
	}
}


void Asteroid::RenderAsteroid() const
{
	Vertex_PCU tempWorldVerts[NUM_ASTEROID_VERTS];
	for (int vertIndex = 0; vertIndex < NUM_ASTEROID_VERTS; ++vertIndex)
	{
		tempWorldVerts[vertIndex] = m_localVerts[vertIndex];
		tempWorldVerts[vertIndex].m_color = m_color;
	}

	TransformVertexArrayXY3D(NUM_ASTEROID_VERTS, &tempWorldVerts[0], 1.f, m_rotateDegree, m_position);
	g_theRenderer->DrawVertexArray(NUM_ASTEROID_VERTS, &tempWorldVerts[0]);
}
void Asteroid::RenderHealthBar() const
{
	DebugDrawLine(Vec2(m_position.x - 2.f, m_position.y + 3.5f), Vec2(m_position.x + 2.f, m_position.y + 3.5f), 0.5f, Rgba8(255, 0, 0, 255));
	DebugDrawLine(Vec2(m_position.x - 2.f, m_position.y + 3.5f), Vec2(m_position.x - 2.f + (m_health * 1.f), m_position.y + 3.5f), 0.5f, Rgba8(0, 255, 0, 255));
}
void Asteroid::Render() const
{
	RenderAsteroid();
	RenderHealthBar();
}



void Asteroid::DebugRender() const
{
	DebugDrawRing(m_position, m_physicsRadius, DEBUG_LINE_THICKNESS, Rgba8(0, 255, 255, 255));
	DebugDrawRing(m_position, m_cosmeticRadius, DEBUG_LINE_THICKNESS, Rgba8(255, 0, 255, 255));

	Vec2 fwdCartPos = Vec2::MakeFromPolarDegrees(m_rotateDegree, m_cosmeticRadius);
	Vec2 fwdPos = Vec2(m_position.x + fwdCartPos.x, m_position.y + fwdCartPos.y);
	DebugDrawLine(m_position, fwdPos, DEBUG_LINE_THICKNESS, Rgba8(255, 0, 0, 255));

	Vec2 velCartPos = Vec2::MakeFromPolarDegrees(Atan2Degrees(m_velocity.y, m_velocity.x), m_velocity.GetLength());
	Vec2 velPos = Vec2(m_position.x + velCartPos.x, m_position.y + velCartPos.y);
	DebugDrawLine(m_position, velPos, DEBUG_LINE_THICKNESS, Rgba8(255, 255, 0, 255));

}

void Asteroid::Die()
{
	m_isDead = true;
	m_isGarbage = true;
	m_game->AddCameraShakeTrauma(0.1f, true);
	m_game->AddCameraShakeTrauma(0.1f, false);
	SoundID die = g_theAudio->CreateOrGetSound("Data/Audio/Die.wav");
	g_theAudio->StartSound(die, false, 0.1f);
	m_game->SpawnNewDebrisCluster(8, m_position, m_velocity, 5.f, m_physicsRadius * DEBRIS_SCALE, m_originalColor);
}

void Asteroid::InitializeLocalVerts()
{

	RandomNumberGenerator m_rng;

	float asteroidLens[NUM_ASTEROID_TRIS] = {};
	for (int asSideIndex = 0; asSideIndex < NUM_ASTEROID_TRIS; ++asSideIndex)
	{
		float randomLen = m_rng.RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
		asteroidLens[asSideIndex] = randomLen;
	}

	float asteroidDegs[NUM_ASTEROID_TRIS] = {};
	float unitDegree = (float) 360.f / NUM_ASTEROID_TRIS;
	for (int asDegIndex = 0; asDegIndex < NUM_ASTEROID_TRIS; ++asDegIndex)
	{
		float degrees = unitDegree * asDegIndex;
		asteroidDegs[asDegIndex] = degrees;
	}
	
	for (int asPosIndex = 0; asPosIndex < NUM_ASTEROID_TRIS; ++asPosIndex)
	{
		int firstVertIndex = asPosIndex;
		int lastVertIndex = (asPosIndex + 1) % NUM_ASTEROID_TRIS;

		Vec3 firstPos = GetAsteroidVertex(asteroidLens[firstVertIndex], asteroidDegs[firstVertIndex]);
		Vec3 lastPos = GetAsteroidVertex(asteroidLens[lastVertIndex], asteroidDegs[lastVertIndex]);

		int first = asPosIndex * 3;
		int second = asPosIndex * 3 + 1;
		int third = asPosIndex * 3 + 2;
		
		m_localVerts[first].m_position = firstPos;
		m_localVerts[third].m_position = Vec3(0.f, 0.f, 0.f);
		m_localVerts[second].m_position = lastPos;

		m_localVerts[first].m_color = m_color;
		m_localVerts[third].m_color = m_color;
		m_localVerts[second].m_color = m_color;
	}

}

Vec3 Asteroid::GetAsteroidVertex(float lens, float degrees)
{
	float indexX = lens * CosDegrees(degrees);
	float indexY = lens * SinDegrees(degrees);
	return Vec3(indexX, indexY, 0);
}