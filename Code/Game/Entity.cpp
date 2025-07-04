#include "Game/Entity.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <math.h>

extern AudioSystem* g_theAudio;
Entity::Entity(Game* owner, Vec2 const& startPos, float orientationDeg, Rgba8 color)
	: m_game(owner)
	, m_position(startPos)
	, m_orientationDegrees(orientationDeg)
	, m_color(color)
{
	m_originalColor = color;
	m_cosmeticRadius = 2.f;
	m_physicsRadius = 1.5f;
	m_health = 1;
	m_hittedTimer = 0.f;
	
}

Entity::~Entity()
{
}

void Entity::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Entity::Render() const
{

}

void Entity::DebugRender() const
{
}

void Entity::Die()
{
	
}

void Entity::BeHitted()
{
	m_health -= 1;
	m_isHitted = true;
	m_color = Rgba8(255, 51, 51, 255);
	m_hittedTimer = 0.f;
	SoundID beHitted = g_theAudio->CreateOrGetSound("Data/Audio/BeHitted.wav");
	g_theAudio->StartSound(beHitted, false, 0.1f);
}

bool Entity::IsOffscreen() const
{
	return (m_position.x < 0.f - m_cosmeticRadius ||
			m_position.x > WORLD_SIZE_X + m_cosmeticRadius ||
			m_position.y < 0.f - m_cosmeticRadius ||
			m_position.y > WORLD_SIZE_Y + m_cosmeticRadius);
}

Vec2 Entity::GetForwardNormal() const
{
	return Vec2(CosDegrees(m_orientationDegrees), SinDegrees(m_orientationDegrees));
}

void Entity::PushOutOfEntity(Entity* other)
{
	PushDiscsOutOfEachOther2D(m_position, m_physicsRadius, other->m_position, other->m_physicsRadius);
}

