#include "Player.h"

using namespace DirectX;

const Player::VehicleInfo Player::NormalTank = 
{
	3.5f,
	6.0f,
	1.7f,
	2.0f,
	2.0f,
	1.2f,
	0.25f,
	4.5f,
	0.75f,
	0.5f,
};

namespace 
{
	Player* g_pplayer = nullptr;
}

Player::Player(const VehicleInfo tankInfo, const XMFLOAT3 direction)
	:
	m_tank(direction, tankInfo)
{
	if (g_pplayer)
		throw std::exception("Player is a singleton!");

	g_pplayer = this;
}

void Player::Init(ID3D11Device* device)
{
	/*
		// 轮子
		DirectX::XMFLOAT3 leftFrontCenterPoint{tankCenterPoint.x - bodyWidth / 2, tankCenterPoint.y - bodyHeight / 2, tankCenterPoint.z + bodyLength / 2};
		DirectX::XMFLOAT3 rightFrontCenterPoint{ tankCenterPoint.x + bodyWidth / 2, tankCenterPoint.y - bodyHeight / 2, tankCenterPoint.z + bodyLength / 2 };
		DirectX::XMFLOAT3 leftBackCenterPoint{ tankCenterPoint.x - bodyWidth / 2, tankCenterPoint.y - bodyHeight / 2, -(tankCenterPoint.z + bodyLength / 2) };
		DirectX::XMFLOAT3 rightBackCenterPoint{ tankCenterPoint.x + bodyWidth / 2, tankCenterPoint.y - bodyHeight / 2, -(tankCenterPoint.z + bodyLength / 2) };
		// 炮管底座
		DirectX::XMFLOAT3 barrelBaseCenterPoint{tankCenterPoint.x, tankCenterPoint.y + bodyHeight /2 + barrelBaseHeight / 2, tankCenterPoint.z };
		// 炮管
		DirectX::XMFLOAT3 barrelCenterPoint{ barrelBaseCenterPoint.x, barrelBaseCenterPoint.y, barrelBaseCenterPoint.z + barrelBaseLength / 2 + barrelLength / 2 };
	 */
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
	const XMFLOAT3 tankCenterPoint = tankBody.GetTransform().GetPosition();
	
	{
		// 炮管底座
		HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.ReleaseAndGetAddressOf()));
		Model barrelBaseModel{ device, Geometry::CreateBox(tankInfo.barrelBaseWidth, tankInfo.barrelBaseLength, tankInfo.barrelBaseHeight) };
		barrelBaseModel.modelParts.front().texDiffuse = texture;

		GameObject& barrelBase = m_tank.barrelBase.self;
		
		barrelBase.SetModel(barrelBaseModel);
		barrelBase.GetTransform().SetRotation(0.0f, -XM_PIDIV2, 0.0f);
		barrelBase.GetTransform().SetPosition(tankCenterPoint.x, tankCenterPoint.y + h2 + tankInfo.barrelBaseHeight / 2, tankCenterPoint.z);

		// 炮管
		HR(CreateDDSTextureFromFile(device, L"Texture\\flare.dds", nullptr, texture.ReleaseAndGetAddressOf()));
		Model barrelModel{ device, Geometry::CreateCylinder(tankInfo.barrelCaliber, tankInfo.barrelLength, 20) };
		barrelModel.modelParts.front().texDiffuse = texture;

		const XMFLOAT3 barrelBaseCenterPoint = barrelBase.GetTransform().GetPosition();
		GameObject& barrel = m_tank.barrelBase.barrel.self;
		barrel.SetModel(barrelModel);
		barrel.GetTransform().SetRotation(XM_PIDIV2, 0.0f, 0.0f);
		barrel.GetTransform().SetPosition(barrelBaseCenterPoint.x, barrelBaseCenterPoint.y, barrelBaseCenterPoint.z + tankInfo.barrelBaseLength / 2 + tankInfo.barrelLength / 2);
	}
	{
		// 轮子
		HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.ReleaseAndGetAddressOf()));
		Model wheelModel{ device, Geometry::CreateCylinder(tankInfo.wheelRadius, tankInfo.wheelLength, 20) };
		wheelModel.modelParts.front().texDiffuse = texture;

		// 左前轮
		GameObject& wheel0 = m_tank.wheels[0].self;
		wheel0.SetModel(wheelModel);
		wheel0.GetTransform().SetRotation(0.0f, 0.0f,  -XM_PIDIV2);
		wheel0.GetTransform().SetPosition(tankCenterPoint.x - w2, tankCenterPoint.y - tankInfo.bodyHeight / 2, tankCenterPoint.z + l2);
		// 右前轮
		GameObject& wheel1 = m_tank.wheels[1].self;
		wheel1.SetModel(wheelModel);
		wheel1.GetTransform().SetRotation(0.0f, 0.0f, XM_PIDIV2);
		wheel1.GetTransform().SetPosition(tankCenterPoint.x + w2, tankCenterPoint.y - tankInfo.bodyHeight / 2, tankCenterPoint.z + l2);
		// 左后轮
		GameObject& wheel2 = m_tank.wheels[2].self;
		wheel2.SetModel(wheelModel);
		wheel2.GetTransform().SetRotation(0.0f, 0.0f, -XM_PIDIV2);
		wheel2.GetTransform().SetPosition(tankCenterPoint.x - w2, tankCenterPoint.y - tankInfo.bodyHeight / 2, tankCenterPoint.z - l2);
		// 右后轮
		GameObject& wheel3 = m_tank.wheels[3].self;
		wheel3.SetModel(wheelModel);
		wheel3.GetTransform().SetRotation(0.0f, 0.0f, XM_PIDIV2);
		wheel3.GetTransform().SetPosition(tankCenterPoint.x + w2, tankCenterPoint.y - tankInfo.bodyHeight / 2, tankCenterPoint.z - l2);
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

Ray Player::Shoot()
{
	return m_tank.Shoot();
}

void Player::Turn(const float d)
{
	m_tank.Turn(d);
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

Player::Tank::Tank(const XMFLOAT3 direction, const VehicleInfo tankInfo)
	:
	tankInfo(tankInfo),
	direction(direction),
	wheels({
		Wheel::WheelPos::LeftFront,
		Wheel::WheelPos::RightFront,
		Wheel::WheelPos::LeftBack,
		Wheel::WheelPos::RightBack
	})
{
}

void Player::Tank::AdjustPosition()
{
	Transform& tankTransform = self.GetTransform();
	XMFLOAT3 adjustedPos{};
	XMStoreFloat3(&adjustedPos, XMVectorClamp(tankTransform.GetPositionXM(), XMVectorSet(-25.0f, 0.5f, -25.0f, 0.0f), XMVectorSet(25.0f, 0.5f, 25.0f, 0.0f)));
	tankTransform.SetPosition(adjustedPos);

	barrelBase.AdjustPosition(*this, tankInfo);
	
	for(auto& wheel : wheels)
	{
		wheel.AdjustPosition(*this, tankInfo);
	}
}

void Player::Tank::Walk(const float d)
{
	Transform& tankTransform = self.GetTransform();
	// 车身移动
	tankTransform.Translate(XMLoadFloat3(&direction), d);
	// 车身顺着移动方向旋转
	XMFLOAT3 up{};
	XMStoreFloat3(&up, tankTransform.GetUpAxisXM());
	XMFLOAT3 look{};
	// 车身是经过了90度旋转的
	XMStoreFloat3(&look, XMVector3Cross(XMLoadFloat3(&direction), tankTransform.GetUpAxisXM()));
	// @TODO 现在是车身直接朝向移动方向,应该缓慢改变才对
	//tankTransform.LookTo(look, up);
	
	// 轮子移动
	for(auto& wheel : wheels)
	{
		wheel.Walk(d, direction);
	}
}

void Player::Tank::Strafe(const float d)
{
	// 轮子转向
	for (auto& wheel : wheels)
	{
		wheel.Strafe(d, direction);
	}
}

Ray Player::Tank::Shoot()
{
	Transform& transform = barrelBase.barrel.self.GetTransform();

	XMFLOAT3 look{};
	// 因为炮管是直立圆柱体向前旋转90度形成,所以上轴指向前方
	XMStoreFloat3(&look, transform.GetUpAxisXM());

	return { transform.GetPosition(), look };
}

void Player::Tank::Turn(const float d)
{
	Transform& barrelBaseTransform = barrelBase.self.GetTransform();
	// 转底座
	barrelBaseTransform.Rotate(XMVectorSet(0.0f, d, 0.0f, 0.0f));
	
	Transform& barrelTransform = barrelBase.barrel.self.GetTransform();
	// 炮管伴随底座一起转
	barrelTransform.RotateAround(barrelBaseTransform.GetPositionXM(), barrelBaseTransform.GetUpAxisXM(), d);

	XMFLOAT3 direction{};
	XMStoreFloat3(&direction, barrelTransform.GetRightAxisXM());
	XMFLOAT3 look{};
	XMStoreFloat3(&look, barrelBaseTransform.GetRightAxisXM());
	// 炮管朝向适应底座朝向
	// @TODO 炮管现在会自身不断旋转(绕Z轴)
	barrelTransform.LookTo(direction, look);
}

Player::Wheel::Wheel(const WheelPos wheelPos)
	: 
	wheelPos(wheelPos)
{
}

void Player::Wheel::Walk(const float d, const XMFLOAT3& direction)
{
	Transform& wheelTransform = self.GetTransform();
	// 移动
	wheelTransform.Translate(XMLoadFloat3(&direction), d);
	// 转起来
	const XMFLOAT3 rotation = wheelTransform.GetRotation();
	wheelTransform.SetRotation(rotation.x + 2.5f * d, rotation.y, rotation.z);
}

void Player::Wheel::Strafe(const float d, XMFLOAT3& direction)
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
	const Transform& tankTransform = body.self.GetTransform();
	// 车身当前方向
	XMFLOAT3 look{};
	XMStoreFloat3(&look, tankTransform.GetRightAxisXM());
	// 车身中心距轮子中心的距离
	const float length = sqrtf(powf(tankInfo.bodyWidth / 2, tankInfo.bodyLength / 2));
	const float offX =  (wheelPos == WheelPos::LeftFront || wheelPos == WheelPos::LeftBack) ? -length * look.x : length * look.x;
	const float offZ = (wheelPos == WheelPos::LeftFront || wheelPos == WheelPos::RightFront) ?  length * look.z : -length * look.z;

	const XMFLOAT3 tankCenterPoint = tankTransform.GetPosition();
	// TODO 方向不对
	self.GetTransform().SetPosition(tankCenterPoint.x + offX, tankCenterPoint.y - tankInfo.bodyHeight / 2, tankCenterPoint.z + offZ);
}

void Player::BarrelBase::AdjustPosition(const Tank& body, const VehicleInfo& tankInfo)
{
	const XMFLOAT3 tankCenter = body.self.GetTransform().GetPosition();
	const float tankHeight = tankInfo.bodyHeight;

	self.GetTransform().SetPosition(tankCenter.x, tankCenter.y + tankHeight / 2 + tankInfo.barrelBaseHeight / 2, tankCenter.z);

	barrel.AdjustPosition(*this, tankInfo);
}

void Player::BarrelBase::Barrel::AdjustPosition(const BarrelBase& body, const VehicleInfo& tankInfo)
{
	const Transform& barrelBaseTransform = body.self.GetTransform();
	// 底座当前角度
	XMFLOAT3 look{};
	XMStoreFloat3(&look, barrelBaseTransform.GetRightAxisXM());
	// 底座中心距炮管中心的距离
	const float length = tankInfo.barrelBaseLength / 2 + tankInfo.barrelLength / 2;
	const XMFLOAT3 barrelBaseCenterPoint = barrelBaseTransform.GetPosition();

	Transform& barrelTransform = self.GetTransform();
	// 炮管适应底座位置位置
	barrelTransform.SetPosition(barrelBaseCenterPoint.x + length * look.x, barrelBaseCenterPoint.y, barrelBaseCenterPoint.z + length * look.z);
}
