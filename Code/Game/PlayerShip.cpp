#include "Game/PlayerShip.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include <math.h>

extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
RandomNumberGenerator m_rng;


PlayerShip::PlayerShip(Game* owner, Vec2 const& pos, float orientationDeg, Rgba8 color, bool isSecondary)
	: Entity(owner, pos, orientationDeg, color)
{
	InitializeVerts(&m_localVerts[0], m_color);
	m_velocity = Vec2(0.f, 0.f);
	m_physicsRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;
	m_health = 1;

	m_isSecondary = isSecondary;
}

PlayerShip::~PlayerShip()
{

}

void PlayerShip::Update(float deltaSeconds)
{
	UpdateTail(deltaSeconds);
	UpdateFromPlayers(deltaSeconds);
	BounceOffWalls();

	m_position += (m_velocity * deltaSeconds);
	
}

void PlayerShip::Render() const
{
	RenderTail();
	RenderShip();
	RenderSkillBar();
}

void PlayerShip::UpdateFromPlayers(float deltaSeconds)
{

	if (!m_game->m_multiplayer)
	{
		UpdateFromKeyboard(deltaSeconds);
		UpdateFromController(deltaSeconds);
	}
	else
	{
		if (m_isSecondary)
		{
			UpdateFromController(deltaSeconds);
		}
		else
		{
			UpdateFromKeyboard(deltaSeconds);
		}
	}
	UpdateSkillInvisible(deltaSeconds);
	m_fireTimer += deltaSeconds;
	m_specialAttackCooldownA += deltaSeconds;
	m_specialAttackCooldownB += deltaSeconds;
}

void PlayerShip::UpdateSkillInvisible(float deltaSeconds)
{

	if (m_isInvisible)
	{
		m_invisibleTimer += deltaSeconds;
		if (m_invisibleTimer >= 3.0f)
		{
			m_isInvisible = false;
			m_invisibleTimer = 0.0f;
			m_invisibleCooldown = 0.0f;
		}
	}
	else
	{
		m_invisibleCooldown += deltaSeconds;
	}
}


void PlayerShip::RenderShip() const
{
	Vertex_PCU worldSpaceVerts[NUM_SHIP_VERTS];

	for (int vertIndex = 0; vertIndex < NUM_SHIP_VERTS; ++vertIndex)
	{
		worldSpaceVerts[vertIndex] = m_localVerts[vertIndex];
		float blink = m_invisibleTimer;
		float alpha;
		if (blink < 1.5f)
		{
			alpha = RangeMapClamped(blink, 0.f, 1.5f, 255.f, 0.f);
		}
		else
		{
			alpha = RangeMapClamped(blink, 1.5f, 3.0f, 0.f, 255.f);
		}
		Rgba8 colorNow = m_color;
		colorNow.a = static_cast<unsigned char> (alpha);
		worldSpaceVerts[vertIndex].m_color = colorNow;

	}
	TransformVertexArrayXY3D(NUM_SHIP_VERTS, &worldSpaceVerts[0], 1.f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, &worldSpaceVerts[0]);
}

void PlayerShip::RenderSkillBar() const
{
	if (!m_isInvisible)
	{
		Rgba8 skillInviColor;
		Rgba8 skillBulletColorA;
		Rgba8 skillBulletColorB;

		if (GetClamped(m_invisibleCooldown, 0.f, 10.f) >= 10.f)
		{
			skillInviColor = m_color;
		}
		else
		{
			skillInviColor = Rgba8(192, 192, 192, 192);
		}
		DebugDrawLine(Vec2(m_position.x - 2.f, m_position.y - 3.5f), Vec2(m_position.x - 2.f + (GetClamped(m_invisibleCooldown, 0.f, 10.f) * 0.4f), m_position.y - 3.5f), 0.5f, skillInviColor);
		
		if (GetClamped(m_specialAttackCooldownA, 0.f, 1.f) >= 1.f)
		{
			skillBulletColorA = Rgba8(255, 205, 0, 255);
		}
		else
		{
			skillBulletColorA = Rgba8(192, 192, 192, 192);
		}
		DebugDrawLine(Vec2(m_position.x - 2.f, m_position.y - 4.5f), Vec2(m_position.x - 2.f + (GetClamped(m_specialAttackCooldownA, 0.f, 1.f) * 4), m_position.y - 4.5f), 0.4f, skillBulletColorA);


		if (GetClamped(m_specialAttackCooldownB, 0.f, 2.f) >= 2.f)
		{
			skillBulletColorB = Rgba8(255, 94, 0, 255);
		}
		else
		{
			skillBulletColorB = Rgba8(192, 192, 192, 192);
		}
		DebugDrawLine(Vec2(m_position.x - 2.f, m_position.y - 5.5f), Vec2(m_position.x - 2.f + (GetClamped(m_specialAttackCooldownB, 0.f, 2.f) * 2), m_position.y - 5.5f), 0.4f, skillBulletColorB);
	}

	
	


}

void PlayerShip::DebugRender() const
{
	DebugDrawRing(m_position, m_physicsRadius, DEBUG_LINE_THICKNESS, Rgba8(0, 255, 255, 255));
	DebugDrawRing(m_position, m_cosmeticRadius, DEBUG_LINE_THICKNESS, Rgba8(255, 0, 255, 255));

	Vec2 fwdCartPos = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_cosmeticRadius);
	Vec2 fwdPos = Vec2(m_position.x + fwdCartPos.x, m_position.y + fwdCartPos.y);
	DebugDrawLine(m_position, fwdPos, DEBUG_LINE_THICKNESS, Rgba8(255, 0, 0, 255));

	Vec2 leftCartPos = Vec2::MakeFromPolarDegrees(m_orientationDegrees + 90.f, m_cosmeticRadius);
	Vec2 leftPos = Vec2(m_position.x + leftCartPos.x, m_position.y + leftCartPos.y);
	DebugDrawLine(m_position, leftPos, DEBUG_LINE_THICKNESS, Rgba8(0, 255, 0, 255));

	Vec2 velCartPos = Vec2::MakeFromPolarDegrees(Atan2Degrees(m_velocity.y, m_velocity.x), m_velocity.GetLength());
	Vec2 velPos = Vec2(m_position.x + velCartPos.x, m_position.y + velCartPos.y);
	DebugDrawLine(m_position, velPos, DEBUG_LINE_THICKNESS, Rgba8(255, 255, 0, 255));
	
	
}

void PlayerShip::RenderTail() const
{
	Vec2 shipDirection = GetForwardNormal();
	Vec2 tailPosition = m_position - shipDirection * 2.f;

	Vec2 flameTip = tailPosition - shipDirection * m_flameLength;
	Vec2 leftBase = tailPosition + Vec2(-shipDirection.y, shipDirection.x);
	Vec2 rightBase = tailPosition + Vec2(shipDirection.y, -shipDirection.x);

	Vertex_PCU tailVertex[3];
	tailVertex[0].m_position = Vec3(flameTip.x, flameTip.y, 0.f);
	tailVertex[2].m_position = Vec3(leftBase.x, leftBase.y, 0.f);
	tailVertex[1].m_position = Vec3(rightBase.x, rightBase.y, 0.f);

	Rgba8 colorNow = m_flameColor;
	colorNow.a = static_cast<unsigned char> (m_flameCurrentAlpha);
	tailVertex[0].m_color = colorNow;
	tailVertex[2].m_color = colorNow;
	tailVertex[1].m_color = colorNow;

	g_theRenderer->DrawVertexArray(3, &tailVertex[0]);
	
}


void PlayerShip::Die()
{
	m_isDead = true;
	if (m_extraLives != 0)
	{
		SoundID die = g_theAudio->CreateOrGetSound("Data/Audio/ShipDie.wav");
		g_theAudio->StartSound(die, false, 0.01f);
	}
	m_game->AddCameraShakeTrauma(1.5f, m_isSecondary);
	m_game->SpawnNewDebrisCluster(20, m_position, m_velocity, 10.f, m_physicsRadius * DEBRIS_SCALE, m_originalColor);
}

void PlayerShip::InitializeVerts(Vertex_PCU* vertsToFillIn, Rgba8 color)
{
	//left wing(A)
	vertsToFillIn[0].m_position	= Vec3(2.f, 1.f, 0.f);
	vertsToFillIn[1].m_position	= Vec3(0.f, 2.f, 0.f);
	vertsToFillIn[2].m_position	= Vec3(-2.f, 1.f, 0.f);
	

	//half main(B)
	vertsToFillIn[3].m_position	= Vec3(0.f, 1.f, 0.f);
	vertsToFillIn[4].m_position	= Vec3(-2.f, 1.f, 0.f);
	vertsToFillIn[5].m_position	= Vec3(-2.f, -1.f, 0.f);
	

	//half main(C)
	vertsToFillIn[6].m_position	= Vec3(0.f, 1.f, 0.f);
	vertsToFillIn[7].m_position	= Vec3(-2.f, -1.f, 0.f);
	vertsToFillIn[8].m_position	= Vec3(0.f, -1.f, 0.f);
	

	//nose(D)
	vertsToFillIn[9].m_position	= Vec3(1.f, 0.f, 0.f);
	vertsToFillIn[10].m_position = Vec3(0.f, 1.f, 0.f);
	vertsToFillIn[11].m_position = Vec3(0.f, -1.f, 0.f);
	

	//right wing(E)
	vertsToFillIn[12].m_position = Vec3(2.f, -1.f, 0.f);
	vertsToFillIn[13].m_position = Vec3(-2.f, -1.f, 0.f);
	vertsToFillIn[14].m_position = Vec3(0.f, -2.f, 0.f);
	
	for (int i = 0; i < NUM_SHIP_VERTS; ++i)
	{
		vertsToFillIn[i].m_color = color;
	}
}

void PlayerShip::UpdateFromKeyboard(float deltaSeconds)
{

	if (IsAlive())
	{
		if (g_theInput->IsKeyDown('S'))
		{
			m_orientationDegrees += PLAYER_SHIP_TURN_SPEED * deltaSeconds;
		}

		if (g_theInput->IsKeyDown('F'))
		{
			m_orientationDegrees -= PLAYER_SHIP_TURN_SPEED * deltaSeconds;
		}

		if (g_theInput->IsKeyDown('E'))
		{
			Vec2 FWD = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
			Vec2 acceleration = FWD * PLAYER_SHIP_ACCELERATION;
			m_velocity += acceleration * deltaSeconds;
			m_velocity.ClampLength(50.f);
			m_thrustFraction += deltaSeconds;
			m_thrustFraction = GetClampedZeroToOne(m_thrustFraction);
		}
		else
		{
			m_thrustFraction -= deltaSeconds;
			m_thrustFraction = GetClampedZeroToOne(m_thrustFraction);
		}

		if (g_theInput->IsKeyDown('J'))
		{
			if (!m_isInvisible && m_fireTimer >= 0.1f)
			{
				Vec2 forwardNormal = GetForwardNormal();
				Vec2 nosePosition = m_position + (forwardNormal * 1.f);

				Vec2 shipVelocity = m_velocity;

				Vec2 bulletRelativeVelocity = forwardNormal * BULLET_SPEED;
				
				Vec2 bulletVelocity = shipVelocity + bulletRelativeVelocity;
				m_game->SpawnBullet(nosePosition, m_orientationDegrees, bulletVelocity);

				m_fireTimer = 0.0f;
				SoundID shootSound = g_theAudio->CreateOrGetSound("Data/Audio/Shoot.wav");
				g_theAudio->StartSound(shootSound, false, 0.5f);
			}
			
		}

		if (g_theInput->WasKeyJustPressed(' ') && m_invisibleCooldown >= 10.0f)
		{
			m_isInvisible = true;
			m_invisibleTimer = 0.0f;
			m_invisibleCooldown = 0.0f;
			SoundID skillInvi = g_theAudio->CreateOrGetSound("Data/Audio/SkillInvi.wav");
			g_theAudio->StartSound(skillInvi, false, .1f);
		}

		if (g_theInput->WasKeyJustPressed('K') && m_specialAttackCooldownA >= 1.f)
		{
			if (!m_isInvisible)
			{
				m_specialAttackCooldownA = 0.0f;
				Vec2 forwardNormal = GetForwardNormal();
				Vec2 nosePosition = m_position + (forwardNormal * 1.f);
				m_game->SpawnBullets(nosePosition, m_orientationDegrees, m_velocity, 12, 60.f);
				SoundID skillBullets = g_theAudio->CreateOrGetSound("Data/Audio/SkillBullets.wav");
				g_theAudio->StartSound(skillBullets, false, .1f);
			}
		}

		if (g_theInput->WasKeyJustPressed('L') && m_specialAttackCooldownB >= 2.f)
		{
			if (!m_isInvisible)
			{
				m_specialAttackCooldownB = 0.0f;
				Vec2 forwardNormal = GetForwardNormal();
				Vec2 nosePosition = m_position + (forwardNormal * 1.f);
				m_game->SpawnBullets(nosePosition, m_orientationDegrees, m_velocity, 36, 360.f);
				SoundID skillBullets = g_theAudio->CreateOrGetSound("Data/Audio/SkillBullets.wav");
				g_theAudio->StartSound(skillBullets, false, .1f);
			}
		}
	} 
	else
	{
		if ((g_theInput->WasKeyJustPressed('N') && m_extraLives > 0))
		{
			Respawn();
		}
	}
	

}

void PlayerShip::UpdateFromController(float deltaSeconds)
{

	XboxController const& controller = g_theInput->GetController(0);

	if (IsAlive() && !m_game->m_multiplayer)
	{
		float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
		if (leftStickMagnitude > 0.f)
		{
			m_thrustFraction = leftStickMagnitude;
			m_orientationDegrees = controller.GetLeftStick().GetOrientationDegrees();
			Vec2 forwardNormal = GetForwardNormal();
			m_velocity += forwardNormal * PLAYER_SHIP_ACCELERATION * m_thrustFraction * deltaSeconds;
			m_velocity.ClampLength(50.f);
		}

		if (controller.IsButtonDown(XboxButtonID::XBOX_BUTTON_A))
		{


			if (!m_isInvisible && m_fireTimer >= 0.1f)
			{
				Vec2 forwardNormal = GetForwardNormal();
				Vec2 nosePosition = m_position + (forwardNormal * 1.f);

				Vec2 shipVelocity = m_velocity;

				Vec2 bulletRelativeVelocity = forwardNormal * BULLET_SPEED;

				Vec2 bulletVelocity = shipVelocity + bulletRelativeVelocity;
				m_game->SpawnBullet(nosePosition, m_orientationDegrees, bulletVelocity);

				m_fireTimer = 0.0f;
				SoundID shootSound = g_theAudio->CreateOrGetSound("Data/Audio/Shoot.wav");
				g_theAudio->StartSound(shootSound, false, 0.5f);
			}
			
		}

		if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_Y) && m_invisibleCooldown >= 10.0f)
		{
			m_isInvisible = true;
			m_invisibleTimer = 0.0f;
			SoundID skillInvi = g_theAudio->CreateOrGetSound("Data/Audio/SkillInvi.wav");
			g_theAudio->StartSound(skillInvi, false, .1f);
		}
		if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_B) && m_specialAttackCooldownA >= 1.f)
		{
			if (!m_isInvisible)
			{
				m_specialAttackCooldownA = 0.0f;
				Vec2 forwardNormal = GetForwardNormal();
				Vec2 nosePosition = m_position + (forwardNormal * 1.f);
				m_game->SpawnBullets(nosePosition, m_orientationDegrees, m_velocity, 12, 60.f);
				SoundID skillBullets = g_theAudio->CreateOrGetSound("Data/Audio/SkillBullets.wav");
				g_theAudio->StartSound(skillBullets, false, .5f);
			}
		}

		if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_X) && m_specialAttackCooldownB >= 2.f)
		{
			if (!m_isInvisible)
			{
				m_specialAttackCooldownB = 0.0f;
				Vec2 forwardNormal = GetForwardNormal();
				Vec2 nosePosition = m_position + (forwardNormal * 1.f);
				m_game->SpawnBullets(nosePosition, m_orientationDegrees, m_velocity, 36, 360.f);
				SoundID skillBullets = g_theAudio->CreateOrGetSound("Data/Audio/SkillBullets.wav");
				g_theAudio->StartSound(skillBullets, false, .5f);
			}
		}

	}
	else
	{
		if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START) && m_extraLives > 0)
		{
			Respawn();
		}
		return;
	}
}

void PlayerShip::UpdateTail(float deltaSeconds)
{

	if (deltaSeconds <= 0.0f)
	{
		return;
	}

	float randomFlicker = m_rng.RollRandomFloatInRange(0.0f, 1.f);
	m_flameColor = Rgba8(255, (unsigned char)(255 * randomFlicker), 0, 255);

	float maxFlameLength = m_thrustFraction * 3.0f;
	m_flameLength = m_rng.RollRandomFloatInRange(0.f, maxFlameLength);

	float blink = m_invisibleTimer;
	float alpha;
	if (blink < 1.5f)
	{
		alpha = RangeMapClamped(blink, 0.f, 1.5f, 255.f, 0.f);
	}
	else
	{
		alpha = RangeMapClamped(blink, 1.5f, 3.0f, 0.f, 255.f);
	}
	m_flameCurrentAlpha = static_cast<unsigned char>(alpha);
}

void PlayerShip::BounceOffWalls()
{
	if (m_position.x < m_physicsRadius)
	{
		m_position.x = m_physicsRadius;
		m_velocity.x *= -1.f;
		if (!m_isInvisible && !m_isDead)
		{
			SoundID collision = g_theAudio->CreateOrGetSound("Data/Audio/Collision.wav");
			g_theAudio->StartSound(collision, false, 0.005f);
		}
		
	}

	if (m_position.x > WORLD_SIZE_X - m_physicsRadius)
	{
		m_position.x = WORLD_SIZE_X - m_physicsRadius;
		m_velocity.x *= -1.f;
		if (!m_isInvisible && !m_isDead)
		{
			SoundID collision = g_theAudio->CreateOrGetSound("Data/Audio/Collision.wav");
			g_theAudio->StartSound(collision, false, 0.005f);
		}
	}

	if (m_position.y < m_physicsRadius)
	{
		m_position.y = m_physicsRadius;
		m_velocity.y *= -1.f;
		if (!m_isInvisible && !m_isDead)
		{
			SoundID collision = g_theAudio->CreateOrGetSound("Data/Audio/Collision.wav");
			g_theAudio->StartSound(collision, false, 0.005f);
		}
	}

	if (m_position.y > WORLD_SIZE_Y - m_physicsRadius)
	{
		m_position.y = WORLD_SIZE_Y - m_physicsRadius;
		m_velocity.y *= -1.f;
		if (!m_isInvisible && !m_isDead)
		{
			SoundID collision = g_theAudio->CreateOrGetSound("Data/Audio/Collision.wav");
			g_theAudio->StartSound(collision, false, 0.005f);
		}
	}
}

void PlayerShip::ShipsCollision()
{
	m_velocity *= -1.f;
	SoundID collision = g_theAudio->CreateOrGetSound("Data/Audio/Collision.wav");
	g_theAudio->StartSound(collision, false, 0.005f);
}



void PlayerShip::Respawn()
{
	if (!m_isSecondary)
	{
		m_position = Vec2(WORLD_CENTER_X - 50.f, WORLD_CENTER_Y);
		m_orientationDegrees = 0.0f;
	}
	else
	{
		m_position = Vec2(WORLD_CENTER_X + 50.f, WORLD_CENTER_Y);
		m_orientationDegrees = 180.0f;
	}
	
	
	m_velocity = Vec2(0.0f, 0.0f);
	m_isDead = false;
	m_health = 1;
	m_extraLives -= 1;
	m_isInvisible = true;
	SoundID respawn = g_theAudio->CreateOrGetSound("Data/Audio/ShipRespawn.wav");
	g_theAudio->StartSound(respawn, false, 0.1f);
}

Vec2 PlayerShip::GetPosition()
{
	return m_position;
}


