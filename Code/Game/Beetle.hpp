#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"
class Game;
class App;

constexpr int NUM_BETTLE_TRIS = 2;
constexpr int NUM_BETTLE_VERTS = 3 * NUM_BETTLE_TRIS;

class Bettle : public Entity
{
public:
	Bettle(Game* owner, Vec2 startPos, float orientationDeg, Rgba8 color);
	~Bettle();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	virtual void Die() override;

	void HandleBeHitted(float deltaSeconds);
	void UpdateOrientationDeg();
	void RenderBettle() const;
	void RenderHealthBar() const;

private:
	void InitializeLocalVerts();

private:
	Vertex_PCU m_localVerts[NUM_BETTLE_VERTS];
};