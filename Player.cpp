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
	// 取得坦克的规格信息
	const VehicleInfo& tankInfo = m_tank.tankInfo;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	// 车身
	HR(CreateDDSTextureFromFile(device, L"Texture\\checkboard.dds", nullptr, texture.GetAddressOf()));
	Model tankModel{ device, Geometry::CreateBox(tankInfo.bodyWidth, tankInfo.bodyLength, tankInfo.bodyHeight) };
	tankModel.modelParts.front().texDiffuse = texture;
	
	GameObject& tankBody = m_tank.self;
	tankBody.SetModel(tankModel);
	tankBody.GetTransform().SetRotation(0.0f, -XM_PIDIV2, 0.0f);
	tankBody.GetTransform().SetPosition(0.0f, 0.5f, 0.0f);

	const float w2 = tankInfo.bodyWidth / 2;
	const float l2 = tankInfo.bodyLength / 2;
	const float h2 = tankInfo.bodyHeight / 2;
	const XMFLOAT3 tankCenter = tankBody.GetTransform().GetPosition();

	// 炮管底座
	{
		HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.ReleaseAndGetAddressOf()));
		Model barrelBaseModel{ device, Geometry::CreateBox(tankInfo.barrelBaseWidth, tankInfo.barrelBaseLength, tankInfo.barrelBaseHeight) };
		barrelBaseModel.modelParts.front().texDiffuse = texture;
		
		GameObject& barrelBase = m_tank.barrelBase.self;
		barrelBase.SetModel(barrelBaseModel);
		barrelBase.GetTransform().SetRotation(0.0f, -XM_PIDIV2, 0.0f);
		barrelBase.GetTransform().SetPosition(tankCenter.x, tankCenter.y + h2 + tankInfo.barrelBaseHeight / 2, tankCenter.z);
	}
	// 炮管
	{
		HR(CreateDDSTextureFromFile(device, L"Texture\\flare.dds", nullptr, texture.ReleaseAndGetAddressOf()));
		Model barrelModel{ device, Geometry::CreateCylinder(tankInfo.barrelCaliber, tankInfo.barrelLength, 20) };
		barrelModel.modelParts.front().texDiffuse = texture;
		
		GameObject& barrel = m_tank.barrelBase.barrel.self;
		barrel.SetModel(barrelModel);
		barrel.GetTransform().SetRotation(XM_PIDIV2, 0.0f, 0.0f);
		barrel.GetTransform().SetPosition(tankCenter.x, tankCenter.y + h2 + tankInfo.barrelBaseHeight / 2 - 0.3f, tankCenter.z + tankInfo.barrelLength / 2);
	}
	// 轮子
	{
		HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.ReleaseAndGetAddressOf()));
		Model wheelModel{ device, Geometry::CreateCylinder(tankInfo.wheelRadius, tankInfo.wheelLength, 20) };
		wheelModel.modelParts.front().texDiffuse = texture;
		
		// 左前轮
		GameObject& wheel0 = m_tank.wheels[0].self;
		wheel0.SetModel(wheelModel);
		wheel0.GetTransform().SetRotation(0.0f, 0.0f,  -XM_PIDIV2);
		wheel0.GetTransform().SetPosition(tankCenter.x - w2, -0.35f, tankCenter.z + l2);
		// 右前轮
		GameObject& wheel1 = m_tank.wheels[1].self;
		wheel1.SetModel(wheelModel);
		wheel1.GetTransform().SetRotation(0.0f, 0.0f, XM_PIDIV2);
		wheel1.GetTransform().SetPosition(tankCenter.x + w2, -0.35f, tankCenter.z + l2);
		// 左后轮
		GameObject& wheel2 = m_tank.wheels[2].self;
		wheel2.SetModel(wheelModel);
		wheel2.GetTransform().SetRotation(0.0f, 0.0f, -XM_PIDIV2);
		wheel2.GetTransform().SetPosition(tankCenter.x - w2, -0.35f, tankCenter.z - l2);
		// 右后轮
		GameObject& wheel3 = m_tank.wheels[3].self;
		wheel3.SetModel(wheelModel);
		wheel3.GetTransform().SetRotation(0.0f, 0.0f, XM_PIDIV2);
		wheel3.GetTransform().SetPosition(tankCenter.x + w2, -0.35f, tankCenter.z - l2);
	}

	m_tank.self.SetDebugObjectName("Tank");
	m_tank.barrelBase.self.SetDebugObjectName("BarrelBase");
	m_tank.barrelBase.barrel.self.SetDebugObjectName("Barrel");
	m_tank.wheels[0].self.SetDebugObjectName("Wheel[0]");
	m_tank.wheels[1].self.SetDebugObjectName("Wheel[1]");
	m_tank.wheels[2].self.SetDebugObjectName("Wheel[2]");
	m_tank.wheels[3].self.SetDebugObjectName("Wheel[3]");
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

void Player::AdjustPosition()
{
	m_tank.AdjustPosition();
}

Transform& Player::GetTransform()
{
	return m_tank.self.GetTransform();
}

const Transform& Player::GetTransform() const
{
	return m_tank.self.GetTransform();
}

void Player::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	m_tank.self.Draw(deviceContext, effect);
	m_tank.barrelBase.self.Draw(deviceContext, effect);
	m_tank.barrelBase.barrel.self.Draw(deviceContext, effect);
	
	for (auto& wheel : m_tank.wheels)
	{
		wheel.self.Draw(deviceContext, effect);
	}
}

Player::Tank::Tank(XMFLOAT3 direction, VehicleInfo tankInfo)
	:
	direction(direction),
	wheels({
		Wheel::WheelPos::LeftFront,
		Wheel::WheelPos::RightFront,
		Wheel::WheelPos::LeftBack,
		Wheel::WheelPos::RightBack
	}),
	tankInfo(tankInfo)
{
}

void Player::Tank::AdjustPosition()
{
	Transform& tankTransform = self.GetTransform();
	XMFLOAT3 adjustedPos{};
	XMStoreFloat3(&adjustedPos, XMVectorClamp(tankTransform.GetPositionXM(), XMVectorSet(-24.0f, 0.5f, -24.0f, 0.0f), XMVectorSet(24.0f, 0.5f, 24.0f, 0.0f)));
	tankTransform.SetPosition(adjustedPos);

	barrelBase.AdjustPosition(*this, tankInfo);
	
	for(auto& wheel : wheels)
	{
		wheel.AdjustPosition(*this, tankInfo);
	}
}

void Player::Tank::Walk(float d)
{
	Transform& tankTransform = self.GetTransform();
	// 车身移动
	tankTransform.Translate(XMLoadFloat3(&direction), d);
	// 车身顺着移动方向旋转
	//transform.RotateAxis(g_XMIdentityR1, XMConvertToRadians(m_direction.x));
	
	// 轮子移动
	for(auto& wheel : wheels)
	{
		wheel.Walk(d, direction);
	}
}

void Player::Tank::Strafe(float d)
{
	// 轮子转向
	for (auto& wheel : wheels)
	{
		wheel.Strafe(d, direction);
	}
}

Player::Wheel::Wheel(WheelPos wheelPos)
	: 
	wheelPos(wheelPos)
{
}

void Player::Wheel::Walk(float d, const XMFLOAT3& direction)
{
	Transform& wheelTransform = self.GetTransform();
	// 移动
	wheelTransform.Translate(XMLoadFloat3(&direction), d);
	// 转起来
	const XMFLOAT3 rotation = wheelTransform.GetRotation();
	wheelTransform.SetRotation(rotation.x + 2.5f * d, rotation.y, rotation.z);
}

void Player::Wheel::Strafe(float d, XMFLOAT3& direction)
{
	// 后轮不转
	if(wheelPos == WheelPos::LeftBack || wheelPos == WheelPos::RightBack)
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
	Transform& wheelTransform = self.GetTransform();
	// 轮胎绕Y轴旋转
	const XMFLOAT3 rotation = wheelTransform.GetRotation();
	wheelTransform.SetRotation(rotation.x, x, rotation.z);
}

void Player::Wheel::AdjustPosition(const Tank& body, const VehicleInfo& tankInfo)
{
	const XMFLOAT3 tankCenter = body.self.GetTransform().GetPosition();
	const float tankWidth = tankInfo.bodyWidth;
	const float tankLength = tankInfo.bodyLength;
	
	const float offW = (wheelPos == WheelPos::LeftFront || wheelPos == WheelPos::LeftBack) ? -tankWidth / 2 : tankWidth / 2;
	const float offL = (wheelPos == WheelPos::LeftBack || wheelPos == WheelPos::RightBack) ? -tankLength / 2 : tankLength / 2;

	self.GetTransform().SetPosition(tankCenter.x + offW, -0.35f, tankCenter.z + offL);
}

void Player::BarrelBase::AdjustPosition(const Tank& body, const VehicleInfo& tankInfo)
{
	const XMFLOAT3 tankCenter = body.self.GetTransform().GetPosition();
	const float tankHeight = tankInfo.bodyHeight;

	const float offH = tankInfo.barrelBaseHeight / 2;

	self.GetTransform().SetPosition(tankCenter.x, tankCenter.y + tankHeight / 2 + offH, tankCenter.z);

	barrel.AdjustPosition(*this, tankInfo);
}

void Player::BarrelBase::Barrel::AdjustPosition(const BarrelBase& body, const VehicleInfo& tankInfo)
{
	const XMFLOAT3 tankCenter = body.self.GetTransform().GetPosition();
	const float tankWidth = tankInfo.barrelBaseWidth;

	const float offL = tankInfo.barrelLength / 2;

	self.GetTransform().SetPosition(tankCenter.x, tankCenter.y, tankCenter.z + tankWidth / 2 + offL);
}
