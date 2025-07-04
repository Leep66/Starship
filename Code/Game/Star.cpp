#include "Star.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <Engine/Core/VertexUtils.hpp>



Star::Star(Game* owner, Vec2 const& startPos, float orientationDeg, Rgba8 color)
	:Entity(owner, startPos, orientationDeg,color)
{
	InitializeLocalVerts();
	
	RandomNumberGenerator m_rng;
	m_scale = m_rng.RollRandomFloatInRange(0.5f, 1.f);
	m_randomBlinkOffset = m_rng.RollRandomFloatInRange(0.5f, 1.f);
}

Star::~Star()
{
}

void Star::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (m_blinkTimer->IsStopped())
	{
		m_blinkTimer->Start();
	}
}

void Star::Render() const
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	Vertex_PCU worldSpaceVerts[NUM_STAR_VERTS];

	float t = m_blinkTimer->GetElapsedFraction();
	float blinkT = fmodf(t + m_randomBlinkOffset, 1.f);
	float sinT = SinDegrees(blinkT * 360.f);
	float alpha = RangeMapClamped(sinT, -1.f, 1.f, 0.f, 255.f);

	for (int vertIndex = 0; vertIndex < NUM_STAR_VERTS; ++vertIndex)
	{
		Rgba8 colorNow = m_color;
		colorNow.a = static_cast<unsigned char>(alpha);
		worldSpaceVerts[vertIndex] = m_localVerts[vertIndex];
		worldSpaceVerts[vertIndex].m_color = colorNow;
	}

	g_theRenderer->DrawVertexArray(NUM_STAR_VERTS, &worldSpaceVerts[0]);
}

void Star::DebugRender() const
{
}

void Star::Die()
{
}

void Star::InitializeLocalVerts()
{
	Vec3 pos3 = Vec3(m_position.x, m_position.y, 0.f);
	Vec3 left = pos3 + Vec3(-3.f, 0.f, 0.f);
	Vec3 leftBot = pos3 + Vec3(-1.f, -1.f, 0.f);
	Vec3 leftTop = pos3 + Vec3(-1.f, 1.f, 0.f);
	Vec3 top = pos3 + Vec3(0.f, 3.f, 0.f);
	Vec3 rightTop = pos3 + Vec3(1.f, 1.f, 0.f);
	Vec3 right = pos3 + Vec3(3.f, 0.f, 0.f);
	Vec3 rightBot = pos3 + Vec3(1.f, -1.f, 0.f);
	Vec3 bot = pos3 + Vec3(0.f, -3.f, 0.f);



	m_localVerts[0].m_position = left;
	m_localVerts[1].m_position = leftBot;
	m_localVerts[2].m_position = leftTop;

	m_localVerts[3].m_position = leftTop;
	m_localVerts[4].m_position = rightTop;
	m_localVerts[5].m_position = top;

	m_localVerts[6].m_position = rightTop;
	m_localVerts[7].m_position = rightBot;
	m_localVerts[8].m_position = right;

	m_localVerts[9].m_position = leftBot;
	m_localVerts[10].m_position = bot;
	m_localVerts[11].m_position = rightBot;

	m_localVerts[12].m_position = leftBot;
	m_localVerts[13].m_position = rightBot;
	m_localVerts[14].m_position = rightTop;

	m_localVerts[15].m_position = leftBot;
	m_localVerts[16].m_position = rightTop;
	m_localVerts[17].m_position = leftTop;


	for (int i = 0; i < NUM_STAR_VERTS; ++i)
	{
		m_localVerts[i].m_color = m_color;
	}
}



