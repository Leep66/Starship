#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"


class Game;

constexpr int NUM_STARS_TRIS = 8;
constexpr int NUM_DEBRIS_VERTS = 3 * NUM_STARS_TRIS;

class Debris : public Entity
{
public:
	Debris(Game* owner, Vec2 const& startPos, Vec2 const& velocity, float radius, Rgba8 color);
	~Debris();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	virtual void Die() override;

private:
	void InitializeLocalVerts();

	Vec3 GetDebrisVertex(float lens, float degrees);

private:
	Vertex_PCU m_localVerts[NUM_DEBRIS_VERTS];
	Rgba8	m_color;
};
