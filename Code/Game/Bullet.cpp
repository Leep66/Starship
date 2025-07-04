#include "Game/Bullet.hpp"
#include "Game/App.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include <math.h>

extern App* g_theApp;
extern Renderer* g_theRenderer;

Bullet::Bullet(Game* owner, Vec2 const& pos, float orientationDeg, Rgba8 color, Vec2 velocity)
	: Entity(owner, pos, orientationDeg, color)
{
	
	m_velocity = velocity;
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_health = 1;

	
	InitializeLocalVerts();
}

Bullet::~Bullet()
{

}

void Bullet::Update(float deltaSeconds)
{

	m_position += (m_velocity * deltaSeconds);

	if (IsOffscreen())
	{
		m_isDead = true;
		m_isGarbage = true;
	}
}

void Bullet::Render() const
{
	Vertex_PCU tempWorldVerts[NUM_BULLET_VERTS];
	for (int vertIndex = 0; vertIndex < NUM_BULLET_VERTS; ++vertIndex)
	{
		tempWorldVerts[vertIndex] = m_localVerts[vertIndex];
	}

	TransformVertexArrayXY3D(NUM_BULLET_VERTS, &tempWorldVerts[0], 1.f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_BULLET_VERTS, &tempWorldVerts[0]);
}

void Bullet::DebugRender() const
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

void Bullet::Die()
{
	m_isDead = true;
	m_isGarbage = true;

	m_game->SpawnNewDebrisCluster(3, m_position, -m_velocity, 30.f, m_physicsRadius * DEBRIS_SCALE, m_originalColor);
}

void Bullet::InitializeLocalVerts()
{
	m_localVerts[0].m_position = Vec3(0.5f, 0.0f, 0.0f);
	m_localVerts[1].m_position = Vec3(0.0f, 0.5f, 0.0f);
	m_localVerts[2].m_position = Vec3(0.0f, -0.5f, 0.0f);
	m_localVerts[0].m_color = m_color;
	m_localVerts[1].m_color = m_color;
	m_localVerts[2].m_color = m_color;


	m_localVerts[3].m_position = Vec3(0.0f, -0.5f, 0.0f);
	m_localVerts[4].m_position = Vec3(0.0f, 0.5f, 0.0f);
	m_localVerts[5].m_position = Vec3(-2.0f, 0.0f, 0.0f);
	m_localVerts[3].m_color = Rgba8(255, 0, 0, 255);
	m_localVerts[4].m_color = Rgba8(255, 0, 0, 255);
	m_localVerts[5].m_color = Rgba8(255, 0, 0, 0);
}


