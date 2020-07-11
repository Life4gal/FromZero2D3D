#include "Player.h"

using namespace DirectX;

namespace 
{
	Player* g_pplayer = nullptr;
}

Player::Player(VehicleInfo tankInfo, XMFLOAT3 direction)
	:
	m_tank(direction, tankInfo)
{
	if (g_pplayer)
		throw std::exception("Player is a singleton!");

	g_pplayer = this;
}

void Player::Init(ID3D11Device* device)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	Material material{};
	material.m_ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	material.m_diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	material.m_specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);

	// 取得坦克的规格信息
	const VehicleInfo& tankInfo = m_tank.m_tankInfo;
	
	// 车身
	HR(CreateDDSTextureFromFile(device, L"Texture\\checkboard.dds", nullptr, texture.GetAddressOf()));
	GameObject& tankBody = m_tank.m_self;
	tankBody.SetBuffer(device, Geometry::CreateBox(tankInfo.m_bodyWidth, tankInfo.m_bodyLength, tankInfo.m_bodyHeight));
	tankBody.GetTransform().SetRotation(0.0f, -XM_PIDIV2, 0.0f);
	// 抬起高度避免深度缓冲区资源争夺
	tankBody.GetTransform().SetPosition(0.0f, 0.5f, 0.0f);
	tankBody.SetTexture(texture.Get());
	tankBody.SetMaterial(material);

	const float w2 = tankInfo.m_bodyWidth / 2;
	const float l2 = tankInfo.m_bodyLength / 2;
	const float h2 = tankInfo.m_bodyHeight / 2;
	const XMFLOAT3 tankCenter = tankBody.GetTransform().GetPosition();

	// 炮管底座
	{
		HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
		GameObject& barrelBase = m_tank.m_barrelBase.m_self;
		barrelBase.SetBuffer(device, Geometry::CreateBox(tankInfo.m_barrelBaseWidth, tankInfo.m_barrelBaseLength, tankInfo.m_barrelBaseHeight));
		barrelBase.GetTransform().SetRotation(0.0f, -XM_PIDIV2, 0.0f);
		barrelBase.GetTransform().SetPosition(tankCenter.x, tankCenter.y + h2 + tankInfo.m_barrelBaseHeight / 2, tankCenter.z);
		barrelBase.SetTexture(texture.Get());
		barrelBase.SetMaterial(material);
	}
	// 炮管
	{
		HR(CreateDDSTextureFromFile(device, L"Texture\\flare.dds", nullptr, texture.GetAddressOf()));
		GameObject& barrel = m_tank.m_barrelBase.m_barrel.m_self;
		barrel.SetBuffer(device, Geometry::CreateCylinder(tankInfo.m_barrelCaliber, tankInfo.m_barrelLength, 20));
		barrel.GetTransform().SetRotation(XM_PIDIV2, 0.0f, 0.0f);
		barrel.GetTransform().SetPosition(tankCenter.x, tankCenter.y + h2 + tankInfo.m_barrelBaseHeight / 2 - 0.3f, tankCenter.z + tankInfo.m_barrelLength / 2);
		barrel.SetTexture(texture.Get());
		barrel.SetMaterial(material);
	}
	// 轮子
	{
		HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
		// 左前轮
		m_tank.m_wheels[0].m_self.GetTransform().SetRotation(0.0f, 0.0f,  -XM_PIDIV2);
		m_tank.m_wheels[0].m_self.GetTransform().SetPosition(tankCenter.x - w2, -0.35f, tankCenter.z + l2);
		// 右前轮
		m_tank.m_wheels[1].m_self.GetTransform().SetRotation(0.0f, 0.0f, XM_PIDIV2);
		m_tank.m_wheels[1].m_self.GetTransform().SetPosition(tankCenter.x + w2, -0.35f, tankCenter.z + l2);
		// 左后轮
		m_tank.m_wheels[2].m_self.GetTransform().SetRotation(0.0f, 0.0f, -XM_PIDIV2);
		m_tank.m_wheels[2].m_self.GetTransform().SetPosition(tankCenter.x - w2, -0.35f, tankCenter.z - l2);
		// 右后轮
		m_tank.m_wheels[3].m_self.GetTransform().SetRotation(0.0f, 0.0f, XM_PIDIV2);
		m_tank.m_wheels[3].m_self.GetTransform().SetPosition(tankCenter.x + w2, -0.35f, tankCenter.z - l2);

		for (auto& wheel : m_tank.m_wheels)
		{
			GameObject& object = wheel.m_self;
			object.SetBuffer(device, Geometry::CreateCylinder(tankInfo.m_wheelRadius, tankInfo.m_wheelLength, 20));
			object.SetMaterial(material);
			object.SetTexture(texture.Get());
		}
	}

	m_tank.m_self.SetDebugObjectName("Tank");
	m_tank.m_barrelBase.m_self.SetDebugObjectName("BarrelBase");
	m_tank.m_barrelBase.m_barrel.m_self.SetDebugObjectName("Barrel");
	m_tank.m_wheels[0].m_self.SetDebugObjectName("Wheel[0]");
	m_tank.m_wheels[1].m_self.SetDebugObjectName("Wheel[1]");
	m_tank.m_wheels[2].m_self.SetDebugObjectName("Wheel[2]");
	m_tank.m_wheels[3].m_self.SetDebugObjectName("Wheel[3]");
}

Player& Player::Get()
{
	if (!g_pplayer)
		throw std::exception("Player needs an instance!");
	
	return *g_pplayer;
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
	return m_tank.m_self.GetTransform();
}

const Transform& Player::GetTransform() const
{
	return m_tank.m_self.GetTransform();
}

void Player::SetMaterial(const Material& material)
{
	m_tank.m_self.SetMaterial(material);
	m_tank.m_barrelBase.m_self.SetMaterial(material);
	m_tank.m_barrelBase.m_barrel.m_self.SetMaterial(material);
	
	for(auto& wheel : m_tank.m_wheels)
	{
		wheel.m_self.SetMaterial(material);
	}
}

void Player::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	m_tank.m_self.Draw(deviceContext, effect);
	m_tank.m_barrelBase.m_self.Draw(deviceContext, effect);
	m_tank.m_barrelBase.m_barrel.m_self.Draw(deviceContext, effect);
	
	for (auto& wheel : m_tank.m_wheels)
	{
		wheel.m_self.Draw(deviceContext, effect);
	}
}

Player::Tank::Tank(XMFLOAT3 direction, VehicleInfo tankInfo)
	:
	m_direction(direction),
	m_wheels({
		Wheel::WheelPos::LeftFront,
		Wheel::WheelPos::RightFront,
		Wheel::WheelPos::LeftBack,
		Wheel::WheelPos::RightBack
	}),
	m_tankInfo(tankInfo)
{
}

void Player::Tank::AdjustPosition()
{
	Transform& tankTransform = m_self.GetTransform();
	XMFLOAT3 adjustedPos{};
	XMStoreFloat3(&adjustedPos, XMVectorClamp(tankTransform.GetPositionXM(), XMVectorSet(-24.0f, 0.5f, -24.0f, 0.0f), XMVectorSet(24.0f, 0.5f, 24.0f, 0.0f)));
	tankTransform.SetPosition(adjustedPos);

	m_barrelBase.AdjustPosition(*this, m_tankInfo);
	
	for(auto& wheel : m_wheels)
	{
		wheel.AdjustPosition(*this, m_tankInfo);
	}
}

void Player::Tank::Walk(float d)
{
	Transform& tankTransform = m_self.GetTransform();
	// 车身移动
	tankTransform.Translate(XMLoadFloat3(&m_direction), d);
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
	Transform& wheelTransform = m_self.GetTransform();
	// 移动
	wheelTransform.Translate(XMLoadFloat3(&direction), d);
	// 转起来
	const XMFLOAT3 rotation = wheelTransform.GetRotation();
	wheelTransform.SetRotation(rotation.x + 2.5f * d, rotation.y, rotation.z);
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
	Transform& wheelTransform = m_self.GetTransform();
	// 轮胎绕Y轴旋转
	const XMFLOAT3 rotation = wheelTransform.GetRotation();
	wheelTransform.SetRotation(rotation.x, x, rotation.z);
}

void Player::Wheel::AdjustPosition(const Tank& body, const VehicleInfo& tankInfo)
{
	const XMFLOAT3 tankCenter = body.m_self.GetTransform().GetPosition();
	const float tankWidth = tankInfo.m_bodyWidth;
	const float tankLength = tankInfo.m_bodyLength;
	
	const float offW = (m_wheelPos == WheelPos::LeftFront || m_wheelPos == WheelPos::LeftBack) ? -tankWidth / 2 : tankWidth / 2;
	const float offL = (m_wheelPos == WheelPos::LeftBack || m_wheelPos == WheelPos::RightBack) ? -tankLength / 2 : tankLength / 2;

	m_self.GetTransform().SetPosition(tankCenter.x + offW, -0.35f, tankCenter.z + offL);
}

void Player::BarrelBase::AdjustPosition(const Tank& body, const VehicleInfo& tankInfo)
{
	const XMFLOAT3 tankCenter = body.m_self.GetTransform().GetPosition();
	const float tankHeight = tankInfo.m_bodyHeight;

	const float offH = tankInfo.m_barrelBaseHeight / 2;

	m_self.GetTransform().SetPosition(tankCenter.x, tankCenter.y + tankHeight / 2 + offH, tankCenter.z);

	m_barrel.AdjustPosition(*this, tankInfo);
}

void Player::BarrelBase::Barrel::AdjustPosition(const BarrelBase& body, const VehicleInfo& tankInfo)
{
	const XMFLOAT3 tankCenter = body.m_self.GetTransform().GetPosition();
	const float tankWidth = tankInfo.m_barrelBaseWidth;

	const float offL = tankInfo.m_barrelLength / 2;

	m_self.GetTransform().SetPosition(tankCenter.x, tankCenter.y, tankCenter.z + tankWidth / 2 + offL);
}
