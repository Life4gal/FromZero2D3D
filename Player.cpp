#include "Player.h"

using namespace DirectX;

namespace 
{
	Player* g_pplayer = nullptr;
}

Player::Player(XMFLOAT3 direction, float bodyWidth, float bodyLength, float bodyHeight, float barrelBaseWidth, float barrelBaseHeight, float barrelLength)
	:
	m_tank(direction, bodyWidth, bodyLength, bodyHeight, barrelBaseWidth, barrelBaseHeight, barrelLength)
{
	if (g_pplayer)
		throw std::exception("Player is a singleton!");

	g_pplayer = this;
}

void Player::init(ID3D11Device* device)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	Material material{};
	material.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);

	// 车身
	HR(CreateDDSTextureFromFile(device, L"Texture\\checkboard.dds", nullptr, texture.GetAddressOf()));
	GameObject& body = m_tank.GetPartObject();
	body.SetBuffer(device, Geometry::CreateBox(m_tank.m_bodyWidth, m_tank.m_bodyLength, m_tank.m_bodyHeight));
	body.GetTransform().SetRotation(0.0f, -XM_PIDIV2, 0.0f);
	// 抬起高度避免深度缓冲区资源争夺
	body.GetTransform().SetPosition(0.0f, 0.5f, 0.0f);
	body.SetTexture(texture.Get());
	body.SetMaterial(material);

	const float w2 = m_tank.m_bodyWidth / 2;
	const float l2 = m_tank.m_bodyLength / 2;
	const float h2 = m_tank.m_bodyHeight / 2;
	const XMFLOAT3 center = body.GetTransform().GetPosition();

	// 炮管底座
	HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	GameObject& barrelBase = m_tank.m_barrelBase.GetPartObject();
	barrelBase.SetBuffer(device, Geometry::CreateBox(m_tank.m_barrelBase.m_bodyWidth, m_tank.m_barrelBase.m_bodyWidth, m_tank.m_barrelBase.m_bodyHeight));
	barrelBase.GetTransform().SetRotation(0.0f, -XM_PIDIV2, 0.0f);
	barrelBase.GetTransform().SetPosition(center.x, center.y + h2 + m_tank.m_barrelBase.m_bodyHeight / 2, center.z);
	barrelBase.SetTexture(texture.Get());
	barrelBase.SetMaterial(material);
	
	// 炮管
	HR(CreateDDSTextureFromFile(device, L"Texture\\flare.dds", nullptr, texture.GetAddressOf()));
	GameObject& barrel = m_tank.m_barrelBase.m_barrel.GetPartObject();
	barrel.SetBuffer(device, Geometry::CreateCylinder(0.5f, m_tank.m_barrelBase.m_barrel.m_length, 20));
	barrel.GetTransform().SetRotation(XM_PIDIV2, 0.0f, 0.0f);
	barrel.GetTransform().SetPosition(center.x, center.y + h2 + m_tank.m_barrelBase.m_bodyHeight / 2 - 0.3f, center.z + m_tank.m_barrelBase.m_barrel.m_length / 2);
	barrel.SetTexture(texture.Get());
	barrel.SetMaterial(material);
	
	// 轮子
	HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	// 左前轮
	m_tank.m_wheels[0].GetPartObject().GetTransform().SetRotation(0.0f, 0.0f,  -XM_PIDIV2);
	m_tank.m_wheels[0].GetPartObject().GetTransform().SetPosition(center.x - w2, -0.35f, center.z + l2);
	// 右前轮
	m_tank.m_wheels[1].GetPartObject().GetTransform().SetRotation(0.0f, 0.0f, XM_PIDIV2);
	m_tank.m_wheels[1].GetPartObject().GetTransform().SetPosition(center.x + w2, -0.35f, center.z + l2);
	// 左后轮
	m_tank.m_wheels[2].GetPartObject().GetTransform().SetRotation(0.0f, 0.0f, -XM_PIDIV2);
	m_tank.m_wheels[2].GetPartObject().GetTransform().SetPosition(center.x - w2, -0.35f, center.z - l2);
	// 右后轮
	m_tank.m_wheels[3].GetPartObject().GetTransform().SetRotation(0.0f, 0.0f, XM_PIDIV2);
	m_tank.m_wheels[3].GetPartObject().GetTransform().SetPosition(center.x + w2, -0.35f, center.z - l2);

	for (auto& wheel : m_tank.m_wheels)
	{
		GameObject& object = wheel.GetPartObject();
		object.SetBuffer(device, Geometry::CreateCylinder(0.75f, 0.5f, 20));
		object.SetMaterial(material);
		object.SetTexture(texture.Get());
	}
}

Player& Player::Get()
{
	if (!g_pplayer)
		throw std::exception("Player needs an instance!");
	return *g_pplayer;
}

void Player::SetDirection(const XMFLOAT3& direction)
{
	m_tank.SetDirection(direction);
}

const XMFLOAT3& Player::GetDirection() const
{
	return m_tank.GetDirection();
}


void Player::Walk(float d)
{
	m_tank.Walk(d);
}

void Player::Strafe(float d)
{
	m_tank.Strafe(d);
}

void Player::AdjustPosition()
{
	m_tank.AdjustPosition();
}

Transform& Player::GetTransform()
{
	return m_tank.GetPartObject().GetTransform();
}

const Transform& Player::GetTransform() const
{
	return m_tank.GetPartObject().GetTransform();
}

void Player::SetMaterial(const Material& material)
{
	m_tank.GetPartObject().SetMaterial(material);
	m_tank.m_barrelBase.GetPartObject().SetMaterial(material);
	m_tank.m_barrelBase.m_barrel.GetPartObject().SetMaterial(material);
	
	for(auto& wheel : m_tank.m_wheels)
	{
		wheel.GetPartObject().SetMaterial(material);
	}
}

void Player::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	m_tank.GetPartObject().Draw(deviceContext, effect);
	m_tank.m_barrelBase.GetPartObject().Draw(deviceContext, effect);
	m_tank.m_barrelBase.m_barrel.GetPartObject().Draw(deviceContext, effect);
	
	for (auto& wheel : m_tank.m_wheels)
	{
		wheel.GetPartObject().Draw(deviceContext, effect);
	}
}

GameObject& Player::VehiclePart::GetPartObject()
{
	return m_part;
}

const GameObject& Player::VehiclePart::GetPartObject() const
{
	return m_part;
}

Player::Tank::Tank(XMFLOAT3 direction, float bodyWidth, float bodyLength, float bodyHeight, float barrelBaseWidth, float barrelBaseHeight, float barrelLength)
	:
	m_direction(direction),
	m_wheels({
		Wheel::WheelPos::LeftFront,
		Wheel::WheelPos::RightFront,
		Wheel::WheelPos::LeftBack,
		Wheel::WheelPos::RightBack
	}),
	m_barrelBase(barrelBaseWidth, barrelBaseHeight, barrelLength),
	m_bodyWidth(bodyWidth),
	m_bodyLength(bodyLength),
	m_bodyHeight(bodyHeight)
	
{
}

void Player::Tank::AdjustPosition()
{
	Transform& transform = GetPartObject().GetTransform();
	XMFLOAT3 adjustedPos{};
	XMStoreFloat3(&adjustedPos, XMVectorClamp(transform.GetPositionXM(), XMVectorSet(-24.0f, 0.5f, -24.0f, 0.0f), XMVectorSet(24.0f, 0.5f, 24.0f, 0.0f)));
	transform.SetPosition(adjustedPos);

	m_barrelBase.AdjustPosition(*this);
	
	for(auto& wheel : m_wheels)
	{
		wheel.AdjustPosition(*this);
	}
}

void Player::Tank::SetDirection(const DirectX::XMFLOAT3& direction)
{
	m_direction = direction;
}

const XMFLOAT3& Player::Tank::GetDirection() const
{
	return m_direction;
}

void Player::Tank::Walk(float d)
{
	Transform& transform = GetPartObject().GetTransform();
	// 车身移动
	transform.Translate(XMLoadFloat3(&m_direction), d);
	// 车身顺着移动方向旋转
	//transform.RotateAxis(g_XMIdentityR1, XMConvertToRadians(m_direction.x));
	
	// 轮子移动
	for(auto& wheel : m_wheels)
	{
		wheel.Walk(d, m_direction);
	}
}

void Player::Tank::Strafe(float d)
{
	// 轮子转向
	for (auto& wheel : m_wheels)
	{
		wheel.Strafe(d, m_direction);
	}
}

Player::Wheel::Wheel(WheelPos wheelPos)
	: 
	m_wheelPos(wheelPos)
{
}

void Player::Wheel::Walk(float d, const XMFLOAT3& direction)
{
	Transform& transform = GetPartObject().GetTransform();
	// 移动
	transform.Translate(XMLoadFloat3(&direction), d);
	// 转起来
	const XMFLOAT3 rotation = transform.GetRotation();
	transform.SetRotation(rotation.x + 2.5f * d, rotation.y, rotation.z);
}

void Player::Wheel::Strafe(float d, XMFLOAT3& direction)
{
	// 后轮不转
	if(m_wheelPos == WheelPos::LeftBack || m_wheelPos == WheelPos::RightBack)
		return;
	// X轴方向分量变化
	float& x = direction.x;
	x += d;
	// 限制旋转范围
	if(x > 0.45f)
	{
		x = 0.45f;
	}
	if (x < -0.45f)
	{
		x = -0.45f;
	}
	Transform& transform = m_part.GetTransform();
	// 轮胎绕Y轴旋转
	const XMFLOAT3 rotation = transform.GetRotation();
	transform.SetRotation(rotation.x, x, rotation.z);
}

void Player::Wheel::AdjustPosition(Tank& body)
{
	Transform& transform = body.GetPartObject().GetTransform();
	
	const XMFLOAT3 targetCenter = transform.GetPosition();
	const float bodyWidth = body.m_bodyWidth;
	const float bodyLength = body.m_bodyLength;
	
	const float offW = (m_wheelPos == WheelPos::LeftFront || m_wheelPos == WheelPos::LeftBack) ? -bodyWidth / 2 : bodyWidth / 2;
	const float offL = (m_wheelPos == WheelPos::LeftBack || m_wheelPos == WheelPos::RightBack) ? -bodyLength / 2 : bodyLength / 2;

	GetPartObject().GetTransform().SetPosition(targetCenter.x + offW, -0.35f, targetCenter.z + offL);
}

Player::BarrelBase::BarrelBase(float bodyWidth, float bodyHeight, float barrelLength)
	:
	m_bodyWidth(bodyWidth),
	m_bodyHeight(bodyHeight),
	m_barrel(barrelLength)
{
}

void Player::BarrelBase::AdjustPosition(Tank& body)
{
	Transform& transform = body.GetPartObject().GetTransform();

	const XMFLOAT3 targetCenter = transform.GetPosition();
	const float bodyHeight = body.m_bodyHeight;

	const float offH = m_bodyHeight / 2;

	GetPartObject().GetTransform().SetPosition(targetCenter.x, targetCenter.y + bodyHeight / 2 + offH, targetCenter.z);

	m_barrel.AdjustPosition(*this);
}

Player::BarrelBase::Barrel::Barrel(float length)
	:
	m_length(length)
{
}

void Player::BarrelBase::Barrel::AdjustPosition(BarrelBase& body)
{
	Transform& transform = body.GetPartObject().GetTransform();

	const XMFLOAT3 targetCenter = transform.GetPosition();
	const float bodyWidth = body.m_bodyWidth;

	const float offL = m_length / 2;

	GetPartObject().GetTransform().SetPosition(targetCenter.x, targetCenter.y, targetCenter.z + bodyWidth / 2 + offL);
}