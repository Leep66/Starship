#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"

class Game;

constexpr int NUM_BULLET_TRIS = 2;
constexpr int NUM_BULLET_VERTS = 3 * NUM_BULLET_TRIS;


class Bullet : public Entity
{
public:
	Bullet(Game* owner, Vec2 const& startPos, float orientationDeg, Rgba8 color, Vec2 velocity);
	~Bullet();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	virtual void Die() override;


private:
	void InitializeLocalVerts();

private:
	Vertex_PCU m_localVerts[NUM_BULLET_VERTS];


};