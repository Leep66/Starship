#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Math/MathUtils.hpp>
#include "Engine/Core/Vertex_PCU.hpp"

class Renderer;

extern Renderer* g_theRenderer;

constexpr int NUM_STARTING_ASTEROIDS = 6;
constexpr int MAX_ASTEROIDS = 400;
constexpr int MAX_BULLETS = 3000;
constexpr int MAX_DEBRIS = 300;
constexpr int MAX_BETTLES = 100;
constexpr int MAX_WASPS = 100;
constexpr float WORLD_SIZE_X = 1000;
constexpr float WORLD_SIZE_Y = 500;
constexpr int MAX_STARS = 100;
constexpr float SCREEN_SIZE_X = 1600;
constexpr float SCREEN_SIZE_Y = 800;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;
constexpr float ASTEROID_SPEED = 10.f;
constexpr float ASTEROID_PHYSICS_RADIUS = 1.6f;
constexpr float ASTEROID_COSMETIC_RADIUS = 2.0f;
constexpr float BULLET_LIFETIME_SECONDS = 2.0f;
constexpr float BULLET_SPEED = 70.f;
constexpr float BULLET_PHYSICS_RADIUS = 0.5f;
constexpr float BULLET_COSMETIC_RADIUS = 2.0f;
constexpr float PLAYER_SHIP_ACCELERATION = 30.f;
constexpr float PLAYER_SHIP_TURN_SPEED = 300.f;
constexpr float PLAYER_SHIP_PHYSICS_RADIUS = 1.75f;
constexpr float PLAYER_SHIP_COSMETIC_RADIUS = 2.25f;
constexpr float BEETLE_SPEED = 15.f;
constexpr float BEETLE_PHYSICS_RADIUS = 1.75f;
constexpr float BEETLE_COSMETIC_RADIUS = 2.25f;
constexpr float WASP_ACCELERATION = 10.f;
constexpr float WASP_PHYSICS_RADIUS = 1.75f;
constexpr float WASP_COSMETIC_RADIUS = 2.25f;
constexpr float DEBUG_LINE_THICKNESS = 0.2f;
constexpr float DEBRIS_LIFETIME_SECONDS = 2.0f;
constexpr float DEBRIS_SCALE = 0.5f;
constexpr float CAM_SHAKE_REDUCTION_PER_SECOND = 0.5f;
constexpr float CAM_SHAKE_MAX = 1.f;





void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);

void DebugDrawLine(Vec2 const& S, Vec2 const& E, float thickness, Rgba8 const& color);