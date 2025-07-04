#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/Timer.hpp"
#include <vector>

class Game;

constexpr int NUM_STAR_TRIS = 6;
constexpr int NUM_STAR_VERTS = 3 * NUM_STAR_TRIS;

class Star : public Entity
{
public:
	Star(Game* owner, Vec2 const& startPos, float orientationDeg, Rgba8 color);
	~Star();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	virtual void Die() override;

	Timer* m_blinkTimer = nullptr;
private:
	void InitializeLocalVerts();

private:
	Vertex_PCU m_localVerts[NUM_STAR_VERTS];
	Rgba8 m_color;
	float m_scale;
	float m_randomBlinkOffset = 0.f;
	
	
};

