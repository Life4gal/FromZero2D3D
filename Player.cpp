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

void Player::init(ID3D11Device* device, float bodyHeight, float bodyLength, float bodyWidth)
{
	m_bodyWidth = bodyWidth;
	m_bodyLength = bodyLength;
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	Material material{};
	material.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);

	// 车身
	HR(CreateDDSTextureFromFile(device, L"Texture\\checkboard.dds", nullptr, texture.GetAddressOf()));
	m_body.SetBuffer(device, Geometry::CreateBox(bodyLength, bodyHeight, bodyWidth));
	m_body.GetTransform().SetRotation(0.0f, -XM_PIDIV2, 0.0f);
	// 抬起高度避免深度缓冲区资源争夺
	m_body.GetTransform().SetPosition(0.0f, 0.5f, 0.0f);
	m_body.SetTexture(texture.Get());
	m_body.SetMaterial(material);

	const float w2 = bodyWidth / 2;
	const float l2 = bodyLength / 2;

	// 轮子
	HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	// 左前轮
	m_wheels[0].GetTransform().SetRotation(0.0f, 0.0f,  -XM_PIDIV2);
	m_wheels[0].GetTransform().SetPosition(-w2, -0.35f, l2);
	// 右前轮
	m_wheels[1].GetTransform().SetRotation(0.0f, 0.0f, XM_PIDIV2);
	m_wheels[1].GetTransform().SetPosition(w2, -0.35f - bodyHeight, l2);
	// 左后轮
	m_wheels[2].GetTransform().SetRotation(0.0f, 0.0f, -XM_PIDIV2);
	m_wheels[2].GetTransform().SetPosition(-w2, -0.35f - bodyHeight, -l2);
	// 右后轮
	m_wheels[3].GetTransform().SetRotation(0.0f, 0.0f, XM_PIDIV2);
	m_wheels[3].GetTransform().SetPosition(w2, -0.35f - bodyHeight, -l2);

	for (auto& wheel : m_wheels)
	{
		wheel.SetBuffer(device, Geometry::CreateCylinder(0.75f, 0.5f, 20));
		wheel.SetMaterial(material);
		wheel.SetTexture(texture.Get());
	}
}

Player& Player::Get()
{
	if (!g_pplayer)
		throw std::exception("Player needs an instance!");
	return *g_pplayer;
}

void Player::Walk(float d)
{
	for(auto& wheel : m_wheels)
	{
		wheel.Walk(d);
	}
	m_body.Walk(d);
}

void Player::Strafe(float d)
{
	for (auto& wheel : m_wheels)
	{
		wheel.Strafe(d);
	}
	m_body.Strafe(d);
}

void Player::AdjustPosition()
{
	m_body.AdjustPosition();
	const XMFLOAT3 position = m_body.GetTransform().GetPosition();
	// 左前轮
	m_wheels[0].AdjustPosition(position, m_bodyWidth, m_bodyLength, Wheel::WheelPos::LeftFront);
	// 右前轮
	m_wheels[1].AdjustPosition(position, m_bodyWidth, m_bodyLength, Wheel::WheelPos::RightFront);
	// 左后轮
	m_wheels[2].AdjustPosition(position, m_bodyWidth, m_bodyLength, Wheel::WheelPos::LeftBack);
	// 右后轮
	m_wheels[3].AdjustPosition(position, m_bodyWidth, m_bodyLength, Wheel::WheelPos::RightBack);
}


Transform& Player::GetTransform()
{
	return m_body.GetTransform();
}

const Transform& Player::GetTransform() const
{
	return m_body.GetTransform();
}

void Player::SetMaterial(const Material& material)
{
	for(auto& wheel : m_wheels)
	{
		wheel.SetMaterial(material);
	}
	m_body.SetMaterial(material);
}

void Player::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	for (auto& wheel : m_wheels)
	{
		wheel.Draw(deviceContext, effect);
	}
	m_body.Draw(deviceContext, effect);
}

void Player::VehiclePart::Walk(float d)
{
	Transform& transform = GetTransform();
	// 右轴叉积上轴并单位向量化得到前轴(Z轴)
	transform.Translate(XMVector3Normalize(XMVector3Cross(transform.GetRightAxisXM(), g_XMIdentityR1)), d);
}

void Player::VehiclePart::Strafe(float d)
{
	Transform& transform = GetTransform();
	transform.Translate(transform.GetRightAxisXM(), d);
}

void Player::Wheel::Walk(float d)
{
	// 轮子转起来
	Transform& transform = GetTransform();
	const XMFLOAT3 rotation = transform.GetRotation();
	transform.SetRotation(rotation.x + 2.5f * d, rotation.y, rotation.z);
	
	VehiclePart::Walk(d);
}

void Player::Wheel::AdjustPosition(XMFLOAT3 targetCenter, float bodyWidth, float bodyLength, WheelPos wheelPos)
{
	const float offW = (wheelPos == WheelPos::LeftFront || wheelPos == WheelPos::LeftBack) ? -bodyWidth / 2 : bodyWidth / 2;
	const float offL = (wheelPos == WheelPos::LeftBack || wheelPos == WheelPos::RightBack) ? -bodyLength / 2 : bodyLength / 2;
	
	GetTransform().SetPosition(targetCenter.x + offW, -0.35f, targetCenter.z + offL);
}

void Player::Body::AdjustPosition()
{
	Transform& transform = GetTransform();
	XMFLOAT3 adjustedPos{};
	XMStoreFloat3(&adjustedPos, XMVectorClamp(transform.GetPositionXM(), XMVectorSet(-8.4f, 0.0f, -8.4f, 0.0f), XMVectorReplicate(8.4f)));
	transform.SetPosition(adjustedPos);
}
