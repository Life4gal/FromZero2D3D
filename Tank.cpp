#include "Tank.h"

using namespace DirectX;

const Tank::VehicleInfo Tank::NormalTank =
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

Tank::Tank(const VehicleInfo tankInfo)
	:
	m_tankInfo(tankInfo)
{
}

void Tank::Init(ID3D11Device* device, Transform transform)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	// 车身
	HR(CreateDDSTextureFromFile(device, L"Texture\\checkboard.dds", nullptr, texture.GetAddressOf()));
	Model tankModel{ device, Geometry::CreateBox(m_tankInfo.bodyWidth, m_tankInfo.bodyLength, m_tankInfo.bodyHeight) };
	tankModel.modelParts.front().texDiffuse = texture;

	GameObject& tank = m_tankRoot;
	tank.SetModel(tankModel);
	tank.GetTransform() = transform;;
	
	{
		// 炮管底座
		HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.ReleaseAndGetAddressOf()));
		Model barrelBaseModel{ device, Geometry::CreateBox(m_tankInfo.barrelBaseWidth, m_tankInfo.barrelBaseLength, m_tankInfo.barrelBaseHeight) };
		barrelBaseModel.modelParts.front().texDiffuse = texture;

		GameObject& barrelBase = m_battery;
		tank.AddChild(&barrelBase);
		
		barrelBase.SetModel(barrelBaseModel);
		barrelBase.GetTransform().SetRotation(0.0f, 0.0f, 0.0f);
		barrelBase.GetTransform().SetPosition(0.0f, m_tankInfo.bodyHeight / 2 + m_tankInfo.barrelBaseHeight / 2, 0.0f);
		
		// 炮管
		HR(CreateDDSTextureFromFile(device, L"Texture\\flare.dds", nullptr, texture.ReleaseAndGetAddressOf()));
		Model barrelModel{ device, Geometry::CreateCylinder(m_tankInfo.barrelCaliber, m_tankInfo.barrelLength, 20) };
		barrelModel.modelParts.front().texDiffuse = texture;

		GameObject& barrel = m_barrel;
		//tank.AddChild(&barrel);
		barrelBase.AddChild(&barrel);
		
		barrel.SetModel(barrelModel);
		barrel.GetTransform().SetRotation(XM_PIDIV2, 0.0f, 0.0f);
		barrel.GetTransform().SetPosition(0.0f, 0.0f, m_tankInfo.barrelBaseLength / 2 + m_tankInfo.barrelLength / 2);
	}
	{
		const float w2 = m_tankInfo.bodyWidth / 2;
		const float l2 = m_tankInfo.bodyLength / 2;
		
		// 轮子
		HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.ReleaseAndGetAddressOf()));
		Model wheelModel{ device, Geometry::CreateCylinder(m_tankInfo.wheelRadius, m_tankInfo.wheelLength, 20) };
		wheelModel.modelParts.front().texDiffuse = texture;

		// 左前轮
		GameObject& wheel0 = m_wheels[0];
		tank.AddChild(&wheel0);
		wheel0.SetModel(wheelModel);
		wheel0.GetTransform().SetRotation(0.0f, 0.0f, -XM_PIDIV2);
		wheel0.GetTransform().SetPosition(-w2, -m_tankInfo.bodyHeight / 2, l2);

		// 右前轮
		GameObject& wheel1 = m_wheels[1];
		tank.AddChild(&wheel1);
		wheel1.SetModel(wheelModel);
		wheel1.GetTransform().SetRotation(0.0f, 0.0f, XM_PIDIV2);
		wheel1.GetTransform().SetPosition(w2, -m_tankInfo.bodyHeight / 2, l2);

		// 左后轮
		GameObject& wheel2 = m_wheels[2];
		tank.AddChild(&wheel2);
		wheel2.SetModel(wheelModel);
		wheel2.GetTransform().SetRotation(0.0f, 0.0f, -XM_PIDIV2);
		wheel2.GetTransform().SetPosition(-w2, -m_tankInfo.bodyHeight / 2, -l2);

		// 右后轮
		GameObject& wheel3 = m_wheels[3];
		tank.AddChild(&wheel3);
		wheel3.SetModel(wheelModel);
		wheel3.GetTransform().SetRotation(0.0f, 0.0f, XM_PIDIV2);
		wheel3.GetTransform().SetPosition(w2, -m_tankInfo.bodyHeight / 2, -l2);
	}

	m_tankRoot.SetDebugObjectName("Tank");
	m_battery.SetDebugObjectName("BarrelBase");
	m_barrel.SetDebugObjectName("Barrel");
	m_wheels[0].SetDebugObjectName("Wheel[0]");
	m_wheels[1].SetDebugObjectName("Wheel[1]");
	m_wheels[2].SetDebugObjectName("Wheel[2]");
	m_wheels[3].SetDebugObjectName("Wheel[3]");
}

void Tank::Walk(const float d)
{
	Transform& tankTransform = m_tankRoot.GetTransform();

	tankTransform.Translate(tankTransform.GetForwardAxisXM(), d);

	// 转轮子
	for(GameObject& wheel : m_wheels)
	{
		wheel.GetTransform().Rotate(XMVectorSet(3.14f * d, 0.0f, 0.0f, 0.0f));
	}
}

void Tank::Strafe(const float d)
{
	XMFLOAT3 direction{};
	XMStoreFloat3(&direction, m_tankRoot.GetTransform().GetForwardAxisXM());
	// X轴方向分量变化
	float& x = direction.x;
	x += d;
	// 限制旋转范围
	if (x > 0.45f)
	{
		x = 0.45f;
	}
	if (x < -0.45f)
	{
		x = -0.45f;
	}
	// 后轮不转
	for(int i = 0; i <= 1; ++i)
	{
		// 轮胎绕Y轴旋转
		m_wheels[i].GetTransform().Rotate(XMVectorSet(0.0f, x, 0.0f, 0.0f));
	}
}

Ray Tank::Shoot()
{
	XMFLOAT3 look{};
	XMStoreFloat3(&look, m_battery.GetTransform().GetForwardAxisXM());
	return { GetBarrelWorldPosition(),  look };
}

void Tank::Turn(const float d)
{
	Transform& batteryTransform = m_battery.GetTransform();
	// 转底座
	batteryTransform.Rotate(XMVectorSet(0.0f, d, 0.0f, 0.0f));

	//m_barrel.GetTransform().RotateAround(batteryTransform.GetPositionXM(), batteryTransform.GetUpAxisXM(), d);
}

XMFLOAT3 Tank::GetPosition() const
{
	return m_tankRoot.GetTransform().GetPosition();
}

void Tank::AdjustPosition(FXMVECTOR minCoordinate, FXMVECTOR maxCoordinate)
{
	Transform& tankTransform = m_tankRoot.GetTransform();

	XMFLOAT3 adjustedPos{};
	XMStoreFloat3(&adjustedPos, XMVectorClamp(tankTransform.GetPositionXM(), minCoordinate, maxCoordinate));
	tankTransform.SetPosition(adjustedPos);
}

void Tank::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	m_tankRoot.Draw(deviceContext, effect);
}

XMFLOAT3 Tank::GetBarrelWorldPosition() const
{
	const Transform& barrelTransform = m_barrel.GetTransform();
	const Transform& batteryTransform = m_battery.GetTransform();
	const Transform& tankTransform = m_tankRoot.GetTransform();

	// 从最顶层子对象到最底层父对象运算: SSSSSSSS * R子T子 * RT * RT * RT * RT * RT * RT * R父T父
	
	const XMMATRIX transform = 
		XMMatrixRotationRollPitchYawFromVector(barrelTransform.GetRotationXM()) *
		XMMatrixTranslationFromVector(barrelTransform.GetPositionXM()) *
		XMMatrixRotationRollPitchYawFromVector(batteryTransform.GetRotationXM()) *
		XMMatrixTranslationFromVector(batteryTransform.GetPositionXM()) *
		XMMatrixRotationRollPitchYawFromVector(tankTransform.GetRotationXM()) *
		XMMatrixTranslationFromVector(tankTransform.GetPositionXM());

	XMFLOAT3 position{};
	XMStoreFloat3(&position, transform.r[3]);

	return position;
}
