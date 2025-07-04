#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

class Game;

constexpr int NUM_ASTEROID_TRIS = 16;
constexpr int NUM_ASTEROID_VERTS = 3 * NUM_ASTEROID_TRIS;
constexpr int NUM_ASTEROID_HEALTH_TRIS = 6;
constexpr int NUM_ASTEROID_HEALTH_VERTS = 3 * NUM_ASTEROID_HEALTH_TRIS;

class Asteroid : public Entity
{
public:
	Asteroid(Game* game, const Vec2& startPos, float orientationDeg, Rgba8 color);
	~Asteroid();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	virtual void Die() override;

	void HandleBeHitted(float deltaSeconds);
	void HandleOffscreen();

	void RenderAsteroid() const;
	void RenderHealthBar() const;

private:
	void InitializeLocalVerts();

	Vec3 GetAsteroidVertex(float lens, float degrees);

private:
	Vertex_PCU m_localVerts[NUM_ASTEROID_VERTS];
	Vertex_PCU m_localHealthBar[NUM_ASTEROID_HEALTH_VERTS];
	float m_rotateDegree;
	

};