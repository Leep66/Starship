#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"

class Game;

constexpr int NUM_SHIP_TRIS = 5;
constexpr int NUM_SHIP_VERTS = 3 * NUM_SHIP_TRIS;

class PlayerShip : public Entity
{
public:
    PlayerShip(Game* game, const Vec2& startPos, float orientationDeg, Rgba8 color, bool isSecondary);
    ~PlayerShip();

    virtual void Update(float deltaSeconds) override;
    virtual void Render() const override;
    virtual void DebugRender() const override;
    void RenderTail() const;
    void RenderShip() const;
    void RenderSkillBar() const;
    virtual void Die() override;
    void Respawn();
    Vec2 GetPosition();
    static void InitializeVerts(Vertex_PCU* vertsToFillIn, Rgba8 color);
    int GetExtraLives() { return m_extraLives; }
    float GetOrientionDegrees() { return m_orientationDegrees; }
    void UpdateSkillInvisible(float deltaSeconds);
    void UpdateFromPlayers(float deltaSeconds);
    void ShipsCollision();

    float m_invisibleTimer = 0.f;
    bool m_isInvisible = false;
    float m_invisibleCooldown = 9.9f;

private:
    
    void UpdateFromKeyboard(float deltaSeconds);
    void UpdateFromController(float deltaSeconds);
    void UpdateTail(float deltaSeconds);
    void BounceOffWalls();
    
    

private:
    Vertex_PCU  m_localVerts[NUM_SHIP_VERTS];
    int m_extraLives = 3;
    float m_thrustFraction;
    float m_fireTimer = -.1f;
    float m_specialAttackCooldownA = 2.f;
    float m_specialAttackCooldownB = 5.f;
    bool m_isSecondary;
    Rgba8 m_flameColor;
    float m_flameLength;
    float m_flameCurrentAlpha;
    
};