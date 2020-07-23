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
	{1.0f, 1.0f, 1.0f},
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.85f + 0.6f/* bodyHeight / 2 + batteryHeight / 2 */, 0.0f},
	0.25f,
	4.5f,
	{1.0f, 1.0f, 1.0f},
	{XM_PIDIV2, 0.0f, 0.0f},
	{0.0f, 0.0f, 1.0f + 2.25f/* batteryLength / 2 + barrelLength / 2 */},
	0.75f,
	0.5f,
	{1.0f, 1.0f, 1.0f},
	{0.0f, 0.0f, -XM_PIDIV2},
	{0.0f, 0.0f, XM_PIDIV2},
	{-1.75f/* -bodyWidth / 2 */, -0.85f/* -bodyHeight / 2 */, 3.0f/* bodyLength / 2 */},
	{1.75f/* bodyWidth / 2 */, -0.85f/* -bodyHeight / 2 */, 3.0f/* bodyLength / 2 */},
	{-1.75f/* -bodyWidth / 2 */, -0.85f/* -bodyHeight / 2 */, -3.0f/* -bodyLength / 2 */},
	{1.75f/* bodyWidth / 2 */, -0.85f/* -bodyHeight / 2 */, -3.0f/* -bodyLength / 2 */}
};

Tank::Tank(const VehicleInfo tankInfo)
	:
	m_tankInfo(tankInfo)
{
}

void Tank::Init(ID3D11Device* device, BasicTransform&& transform)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	// 车身
	GameObject& tank = m_tankMainBody;
	{
		HR(CreateDDSTextureFromFile(device, L"Texture\\checkboard.dds", nullptr, texture.GetAddressOf()));
		Model tankModel{ device, Geometry::CreateBox(m_tankInfo.bodyWidth, m_tankInfo.bodyLength, m_tankInfo.bodyHeight) };
		tankModel.modelParts.front().texDiffuse = texture;
		tank.SetModel(std::move(tankModel));
		tank.GetTransform() = transform;
	}
	{
		// 炮管底座
		GameObject& battery = m_battery;
		tank.AddChild(&battery);
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model barrelBaseModel{ device, Geometry::CreateBox(m_tankInfo.batteryWidth, m_tankInfo.batteryLength, m_tankInfo.batteryHeight) };
			barrelBaseModel.modelParts.front().texDiffuse = texture;
			battery.SetModel(std::move(barrelBaseModel));
		}
		{
			BasicTransform& batteryTransform = battery.GetTransform();
			batteryTransform.SetScale(m_tankInfo.batteryScale);
			batteryTransform.SetRotation(m_tankInfo.batteryRotation);
			batteryTransform.SetPosition(m_tankInfo.batteryPosition);
		}

		// 炮管
		GameObject& barrel = m_barrel;
		battery.AddChild(&barrel);
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\flare.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model barrelModel{ device, Geometry::CreateCylinder(m_tankInfo.barrelCaliber, m_tankInfo.barrelLength, 20) };
			barrelModel.modelParts.front().texDiffuse = texture;
			barrel.SetModel(std::move(barrelModel));
		}
		{
			BasicTransform& barrelTransform = barrel.GetTransform();
			barrelTransform.SetScale(m_tankInfo.barrelScale);
			barrelTransform.SetRotation(m_tankInfo.barrelRotation);
			barrelTransform.SetPosition(m_tankInfo.barrelPosition);
		}
	}
	{
		// 轮子
		HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.ReleaseAndGetAddressOf()));
		Model wheelModel{ device, Geometry::CreateCylinder(m_tankInfo.wheelRadius, m_tankInfo.wheelLength, 20) };
		wheelModel.modelParts.front().texDiffuse = texture;
		{
			// 左前轮
			GameObject& wheel0 = m_wheels[0];
			tank.AddChild(&wheel0);
			
			wheel0.SetModel(wheelModel);
			{
				BasicTransform& wheel0Transform = wheel0.GetTransform();
				wheel0Transform.SetScale(m_tankInfo.wheelScale);
				wheel0Transform.SetRotation(m_tankInfo.wheelLeftRotation);
				wheel0Transform.SetPosition(m_tankInfo.wheelLeftFrontPosition);
			}
		}
		{
			// 右前轮
			GameObject& wheel1 = m_wheels[1];
			tank.AddChild(&wheel1);
			
			wheel1.SetModel(wheelModel);
			{
				BasicTransform& wheel1Transform = wheel1.GetTransform();
				wheel1Transform.SetScale(m_tankInfo.wheelScale);
				wheel1Transform.SetRotation(m_tankInfo.wheelRightRotation);
				wheel1Transform.SetPosition(m_tankInfo.wheelRightFrontPosition);
			}
		}
		{
			// 左后轮
			GameObject& wheel2 = m_wheels[2];
			tank.AddChild(&wheel2);
			
			wheel2.SetModel(wheelModel);
			{
				BasicTransform& wheel2Transform = wheel2.GetTransform();
				wheel2Transform.SetScale(m_tankInfo.wheelScale);
				wheel2Transform.SetRotation(m_tankInfo.wheelLeftRotation);
				wheel2Transform.SetPosition(m_tankInfo.wheelLeftBackPosition);
			}
		}
		{
			// 右后轮
			GameObject& wheel3 = m_wheels[3];
			tank.AddChild(&wheel3);
			
			wheel3.SetModel(wheelModel);
			{
				BasicTransform& wheel3Transform = wheel3.GetTransform();
				wheel3Transform.SetScale(m_tankInfo.wheelScale);
				wheel3Transform.SetRotation(m_tankInfo.wheelRightRotation);
				wheel3Transform.SetPosition(m_tankInfo.wheelRightBackPosition);
			}
		}
	}

	m_tankMainBody.SetDebugObjectName("Tank");
	m_battery.SetDebugObjectName("BarrelBase");
	m_barrel.SetDebugObjectName("Barrel");
	m_wheels[0].SetDebugObjectName("Wheel[0]");
	m_wheels[1].SetDebugObjectName("Wheel[1]");
	m_wheels[2].SetDebugObjectName("Wheel[2]");
	m_wheels[3].SetDebugObjectName("Wheel[3]");
}

void Tank::Walk(const float d)
{
	BasicTransform& tankTransform = m_tankMainBody.GetTransform();

	tankTransform.Translate(tankTransform.GetForwardAxisVector(), d);

	// 转轮子
	for(GameObject& wheel : m_wheels)
	{
		wheel.GetTransform().Rotate(XMVectorSet(3.14f * d, 0.0f, 0.0f, 0.0f));
	}
}

void Tank::Strafe(const float d)
{
	BasicTransform& tankTransform = m_tankMainBody.GetTransform();
	// 转车身
	// @TODO 找一个比较合理的旋转速度
	tankTransform.Rotate(XMVectorSet(0.0f, 0.1f * d, 0.0f, 0.0f));
	
	// 后轮不转
	// 轮胎绕Y轴旋转
	// const XMVECTOR upAxis = tankTransform.GetUpAxisVector();
	// @TODO 限制旋转范围
	// m_wheels[0].GetTransform().RotateAround(XMLoadFloat3(&m_tankInfo.wheelLeftFrontPosition), upAxis, d);
	// m_wheels[1].GetTransform().RotateAround(XMLoadFloat3(&m_tankInfo.wheelRightFrontPosition), upAxis, d);
}

Ray Tank::Shoot() const
{
	XMMATRIX barrelLocalToWorldMatrix = GetBarrelLocalToWorldMatrix();
	
	XMFLOAT3 position{};
	XMStoreFloat3(&position, barrelLocalToWorldMatrix.r[3]);
	return { position,  barrelLocalToWorldMatrix.r[1] };
}

void Tank::Turn(const float d)
{
	BasicTransform& batteryTransform = m_battery.GetTransform();
	// 转底座
	// @TODO 找一个比较合理的旋转速度
	batteryTransform.Rotate(XMVectorSet(0.0f, 0.25f * d, 0.0f, 0.0f));
}

XMFLOAT3 Tank::GetPosition() const
{
	return m_tankMainBody.GetTransform().GetPositionFloat3();
}

void Tank::AdjustPosition(FXMVECTOR minCoordinate, FXMVECTOR maxCoordinate)
{
	BasicTransform& tankTransform = m_tankMainBody.GetTransform();

	XMFLOAT3 adjustedPos{};
	XMStoreFloat3(&adjustedPos, XMVectorClamp(tankTransform.GetPositionVector(), minCoordinate, maxCoordinate));
	tankTransform.SetPosition(adjustedPos);
}

void Tank::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	m_tankMainBody.Draw(deviceContext, effect);
}

XMMATRIX Tank::GetBarrelLocalToWorldMatrix() const
{
	const BasicTransform& barrelTransform = m_barrel.GetTransform();
	const BasicTransform& batteryTransform = m_battery.GetTransform();
	const BasicTransform& tankTransform = m_tankMainBody.GetTransform();

	// 从最顶层子对象到最底层父对象运算: SSSSSSSS * R子T子 * RT * RT * RT * RT * RT * RT * R父T父
	
	return
		barrelTransform.GetScaleMatrix() *
		batteryTransform.GetScaleMatrix() *
		tankTransform.GetScaleMatrix() *

		barrelTransform.GetRotationTranslationMatrix() *
		batteryTransform.GetRotationTranslationMatrix() *
		tankTransform.GetRotationTranslationMatrix();
}
