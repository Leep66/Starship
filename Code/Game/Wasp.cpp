#include "Wasp.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/App.hpp"
extern App* g_theApp;
extern AudioSystem* g_theAudio;

Wasp::Wasp(Game* owner, Vec2 startPos, float orientationDeg, Rgba8 color)
	:Entity(owner, startPos, orientationDeg, color)
{
	m_physicsRadius = WASP_PHYSICS_RADIUS;
	m_cosmeticRadius = WASP_COSMETIC_RADIUS;
	m_health = 2;
	InitializeLocalVerts();
}

Wasp::~Wasp()
{
}

void Wasp::Update(float deltaSeconds)
{

	UpdateOrientationDegAndVel(deltaSeconds);
	HandleBeHitted(deltaSeconds);
	m_position += (m_velocity * deltaSeconds);
}


void Wasp::UpdateOrientationDegAndVel(float deltaSeconds)
{

	if (!m_game->m_multiplayer)
	{
		Vec2 fwdDegrees = m_game->GetPlayership(0)->GetPosition() - m_position;
		if (m_game->GetPlayership(0)->IsAlive() && !m_game->GetPlayership(0)->m_isInvisible)
		{
			m_orientationDegrees = Atan2Degrees(fwdDegrees.y, fwdDegrees.x);
			Vec2 acceleration = Vec2::MakeFromPolarDegrees(m_orientationDegrees) * WASP_ACCELERATION;
			m_velocity += acceleration * deltaSeconds;
			m_velocity.ClampLength(40.f);
		}
	}
	else
	{
		PlayerShip* playerA = m_game->GetPlayership(0);
		PlayerShip* playerB = m_game->GetPlayership(1);

		bool playerAIsValid = playerA != nullptr && playerA->IsAlive() && !playerA->m_isInvisible;
		bool playerBIsValid = playerB != nullptr && playerB->IsAlive() && !playerB->m_isInvisible;

		if (playerAIsValid && playerBIsValid)
		{
			float distToPlayer0 = (playerA->GetPosition() - m_position).GetLengthSquared();
			float distToPlayer1 = (playerB->GetPosition() - m_position).GetLengthSquared();

			if (distToPlayer0 < distToPlayer1)
			{
				Vec2 fwdDegrees = playerA->GetPosition() - m_position;
				m_orientationDegrees = Atan2Degrees(fwdDegrees.y, fwdDegrees.x);
			}
			else
			{
				Vec2 fwdDegrees = playerB->GetPosition() - m_position;
				m_orientationDegrees = Atan2Degrees(fwdDegrees.y, fwdDegrees.x);
			}
		}
		else if (playerAIsValid)
		{
			Vec2 fwdDegrees = playerA->GetPosition() - m_position;
			m_orientationDegrees = Atan2Degrees(fwdDegrees.y, fwdDegrees.x);
		}
		else if (playerBIsValid)
		{
			Vec2 fwdDegrees = playerB->GetPosition() - m_position;
			m_orientationDegrees = Atan2Degrees(fwdDegrees.y, fwdDegrees.x);
		}
		Vec2 acceleration = Vec2::MakeFromPolarDegrees(m_orientationDegrees) * WASP_ACCELERATION;
		m_velocity += acceleration * deltaSeconds;
		m_velocity.ClampLength(40.f);
	}
	
}

void Wasp::HandleBeHitted(float deltaSeconds)
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

void Wasp::Render() const
{
	RenderWasp();
	RenderHealthBar();
}

void Wasp::RenderWasp() const
{
	Vertex_PCU worldSpaceVerts[NUM_WASP_VERTS];

	for (int vertIndex = 0; vertIndex < NUM_WASP_VERTS; ++vertIndex)
	{
		worldSpaceVerts[vertIndex] = m_localVerts[vertIndex];
		worldSpaceVerts[vertIndex].m_color = m_color;
	}
	TransformVertexArrayXY3D(NUM_WASP_VERTS, &worldSpaceVerts[0], 1.f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_WASP_VERTS, &worldSpaceVerts[0]);
}

void Wasp::RenderHealthBar() const
{
	DebugDrawLine(Vec2(m_position.x - 2.f, m_position.y + 3.5f), Vec2(m_position.x + 2.f, m_position.y + 3.5f), 0.5f, Rgba8(255, 0, 0, 255));
	DebugDrawLine(Vec2(m_position.x - 2.f, m_position.y + 3.5f), Vec2(m_position.x - 2.f + (m_health * 2.f), m_position.y + 3.5f), 0.5f, Rgba8(0, 255, 0, 255));
}

void Wasp::DebugRender() const
{
	DebugDrawRing(m_position, m_physicsRadius, DEBUG_LINE_THICKNESS, Rgba8(0, 255, 255, 255));

	DebugDrawRing(m_position, m_cosmeticRadius, DEBUG_LINE_THICKNESS, Rgba8(255, 0, 255, 255));

	Vec2 fwdCartPos = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_cosmeticRadius);
	Vec2 fwdPos = Vec2(m_position.x + fwdCartPos.x, m_position.y + fwdCartPos.y);
	DebugDrawLine(m_position, fwdPos, DEBUG_LINE_THICKNESS, Rgba8(255, 0, 0, 255));

	Vec2 leftCartPos = Vec2::MakeFromPolarDegrees(m_orientationDegrees + 90.f, m_cosmeticRadius);
	Vec2 leftPos = Vec2(m_position.x + leftCartPos.x, m_position.y + leftCartPos.y);
	DebugDrawLine(m_position, leftPos, DEBUG_LINE_THICKNESS, Rgba8(0, 255, 0, 255));

	Vec2 velCartPos = Vec2::MakeFromPolarDegrees(Atan2Degrees(m_velocity.y, m_velocity.x), m_velocity.GetLength());
	Vec2 velPos = Vec2(m_position.x + velCartPos.x, m_position.y + velCartPos.y);
	DebugDrawLine(m_position, velPos, DEBUG_LINE_THICKNESS, Rgba8(255, 255, 0, 255));
}

void Wasp::Die()
{
	m_isDead = true;
	m_isGarbage = true;
	SoundID die = g_theAudio->CreateOrGetSound("Data/Audio/Die.wav");
	g_theAudio->StartSound(die, false, 0.1f);
	m_game->AddCameraShakeTrauma(0.1f, true);
	m_game->AddCameraShakeTrauma(0.1f, false);
	m_game->SpawnNewDebrisCluster(8, m_position, m_velocity, 10.f, m_physicsRadius * DEBRIS_SCALE, m_originalColor);
}

void Wasp::InitializeLocalVerts()
{
	m_localVerts[0].m_position = Vec3(3.f, 0.f, 0.f);
	m_localVerts[2].m_position = Vec3(0.5f, 0.f, 0.f);
	m_localVerts[1].m_position = Vec3(-0.5f, 2.f, 0.f);

	m_localVerts[3].m_position = Vec3(3.f, 0.f, 0.f);
	m_localVerts[4].m_position = Vec3(0.5f, 0.f, 0.f);
	m_localVerts[5].m_position = Vec3(-0.5f, -2.f, 0.f);

	for (int i = 0; i < NUM_WASP_VERTS; ++i)
	{
		m_localVerts[i].m_color = m_color;
	}
}
