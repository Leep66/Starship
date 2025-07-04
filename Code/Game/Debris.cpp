#include "Debris.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Clock.hpp"
#include <Engine/Core/VertexUtils.hpp>

Debris::Debris(Game* owner, Vec2 const& startPos, Vec2 const& velocity, float radius, Rgba8 color)
	: Entity(owner, startPos, owner->m_rng->RollRandomFloatInRange(0.f, 360.f), color)
{
	m_cosmeticRadius = radius * 1.5f;
	m_physicsRadius = radius * 0.5f;
	m_health = 1;
	m_velocity = velocity;
	m_angularVeclocity = owner->m_rng->RollRandomFloatInRange(-200.f, 200.f);
	m_velocity = Vec2(owner->m_rng->RollRandomFloatInRange(-20.f, 20.f), owner->m_rng->RollRandomFloatInRange(-20.f, 20.f));
	m_color = color;
	InitializeLocalVerts();
}

Debris::~Debris()
{
}

void Debris::Update(float deltaSeconds)
{

	m_ageInSeconds += deltaSeconds;
	m_position += (m_velocity * deltaSeconds);
	m_orientationDegrees += m_angularVeclocity * deltaSeconds;
	

	if (IsOffscreen() || m_ageInSeconds > DEBRIS_LIFETIME_SECONDS)
	{
		Die();
	}

	
}

void Debris::Render() const
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	Vertex_PCU tempWorldVerts[NUM_DEBRIS_VERTS];
	for (int vertIndex = 0; vertIndex < NUM_DEBRIS_VERTS; ++vertIndex)
	{
		tempWorldVerts[vertIndex] = m_localVerts[vertIndex];

		float alpha = RangeMapClamped(m_ageInSeconds, 0.f, 2.f, 127.f, 0.f);
		Rgba8 colorNow = m_color;
		colorNow.a = static_cast<unsigned char> (alpha);
		
		tempWorldVerts[vertIndex].m_color = colorNow;
	}

	TransformVertexArrayXY3D(NUM_DEBRIS_VERTS, &tempWorldVerts[0], 1.f, m_orientationDegrees, m_position);
	
	
	g_theRenderer->DrawVertexArray(NUM_DEBRIS_VERTS, &tempWorldVerts[0]);
}

void Debris::DebugRender() const
{
}

void Debris::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}

void Debris::InitializeLocalVerts()
{
	RandomNumberGenerator m_rng;
	float debrisLens[NUM_STARS_TRIS] = {};
	for (int debSideIndex = 0; debSideIndex < NUM_STARS_TRIS; ++debSideIndex)
	{
		float randomLen = m_rng.RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
		debrisLens[debSideIndex] = randomLen;
	}

	float debrisDegs[NUM_STARS_TRIS] = {};
	float unitDegree = (float)360.f / NUM_STARS_TRIS;
	for (int debrisDegIndex = 0; debrisDegIndex < NUM_STARS_TRIS; ++debrisDegIndex)
	{
		float degrees = unitDegree * debrisDegIndex;
		debrisDegs[debrisDegIndex] = degrees;
	}

	for (int debrisPosIndex = 0; debrisPosIndex < NUM_STARS_TRIS; ++debrisPosIndex)
	{
		int firstVertIndex = debrisPosIndex;
		int lastVertIndex = (debrisPosIndex + 1) % NUM_STARS_TRIS;

		Vec3 firstPos = GetDebrisVertex(debrisLens[firstVertIndex], debrisDegs[firstVertIndex]);
		Vec3 lastPos = GetDebrisVertex(debrisLens[lastVertIndex], debrisDegs[lastVertIndex]);

		int first = debrisPosIndex * 3;
		int second = debrisPosIndex * 3 + 1;
		int third = debrisPosIndex * 3 + 2;

		m_localVerts[first].m_position = firstPos;
		m_localVerts[third].m_position = Vec3(0.f, 0.f, 0.f);
		m_localVerts[second].m_position = lastPos;

		m_localVerts[first].m_color = m_color;
		m_localVerts[third].m_color = m_color;
		m_localVerts[second].m_color = m_color;
	}
}

Vec3 Debris::GetDebrisVertex(float lens, float degrees)
{
	float indexX = lens * CosDegrees(degrees);
	float indexY = lens * SinDegrees(degrees);
	return Vec3(indexX, indexY, 0);
}
