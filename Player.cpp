#include "Player.h"

using namespace DirectX;

namespace 
{
	Player* g_pplayer = nullptr;
}

Player::Player()
{
	if (g_pplayer)
		throw std::exception("Player is a singleton!");

	g_pplayer = this;
}

void Player::Init(ID3D11Device* device)
{
	m_tank.Init(device);
}

Player& Player::Get()
{
	if (!g_pplayer)
		throw std::exception("Player needs an instance!");
	
	return *g_pplayer;
}

void Player::Walk(const float d)
{
	m_tank.Walk(d);
}

void Player::Strafe(const float d)
{
	m_tank.Strafe(d);
}

Ray Player::Shoot() const
{
	return m_tank.Shoot();
}

void Player::Turn(const float d)
{
	m_tank.Turn(d);
}

void XM_CALLCONV Player::AdjustPosition(FXMVECTOR minCoordinate, FXMVECTOR maxCoordinate)
{
	m_tank.AdjustPosition(minCoordinate, maxCoordinate);
}

XMFLOAT3 Player::GetPosition() const
{
	return m_tank.GetPosition();
}

void Player::SetPosition(const XMFLOAT3& position)
{
	m_tank.SetPosition(position);
}

void Player::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	m_tank.Draw(deviceContext, effect);
}