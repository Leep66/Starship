#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/Entity.hpp"

class Game;

constexpr int NUM_WASP_TRIS = 2;
constexpr int NUM_WASP_VERTS = 3 * NUM_WASP_TRIS;

class Wasp : public Entity
{
public:
	Wasp(Game* owner, Vec2 startPos, float orientationDeg, Rgba8 color);
	~Wasp();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	virtual void Die() override;

	void HandleBeHitted(float deltaSeconds);
	void UpdateOrientationDegAndVel(float deltaSeconds);
	void RenderWasp() const;
	void RenderHealthBar() const;

private:
	void InitializeLocalVerts();

private:
	Vertex_PCU m_localVerts[NUM_WASP_VERTS];
	
};