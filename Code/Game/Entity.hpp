#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Clock.hpp"

class Game;

class Entity
{
public:
	Entity(Game* game, const Vec2& startPos, float orientationDeg, Rgba8 color);
	virtual ~Entity();

	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;
	virtual void DebugRender() const = 0;
	virtual void Die();

	void BeHitted();

	bool IsOffscreen() const;
	Vec2 GetForwardNormal() const;

	void PushOutOfEntity(Entity* other);
	bool IsAlive() const { return !m_isDead; }
	Game* GetGame() const { return m_game; }

	Vec2 GetPosition() const { return m_position; }
	float GetPhysicsRadius() const { return m_physicsRadius; }
	int GetHealth() const { return m_health; }
	bool GetIsGarbage() { return m_isGarbage; }
	
protected:
	Game*	m_game					= nullptr;

	Vec2	m_position;
	Vec2	m_velocity;
	Rgba8	m_color;
	Rgba8   m_originalColor;
	

	float	m_orientationDegrees	= 0.f;
	float	m_angularVeclocity		= 0.f;
	float	m_physicsRadius;
	float	m_cosmeticRadius;

	float	m_ageInSeconds = 0.f;
	float	m_hittedTimer;
	float	m_hitColorDuration = 0.2f;
	
	int		m_health;
	bool	m_isDead				= false;
	bool	m_isGarbage				= false;
	bool	m_isHitted				= false;
	
	Rgba8	m_hitColor				= Rgba8(255, 51, 51, 255);
	
};