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
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.85f + 0.6f/* bodyHeight / 2 + batteryHeight / 2 */, 0.0f},
	0.25f,
	4.5f,
	{XM_PIDIV2, 0.0f, 0.0f},
	{0.0f, 0.0f, 1.0f + 2.25f/* batteryLength / 2 + barrelLength / 2 */},
	0.75f,
	0.5f,
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

void Tank::Init(ID3D11Device* device, Transform&& transform)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	// 车身
	HR(CreateDDSTextureFromFile(device, L"Texture\\checkboard.dds", nullptr, texture.GetAddressOf()));
	Model tankModel{ device, Geometry::CreateBox(m_tankInfo.bodyWidth, m_tankInfo.bodyLength, m_tankInfo.bodyHeight) };
	tankModel.modelParts.front().texDiffuse = texture;

	GameObject& tank = m_tankMainBody;
	tank.SetModel(std::move(tankModel));
	tank.GetTransform() = transform;
	{
		// 炮管底座
		HR(CreateDDSTextureFromFile(device, L"Texture\\WoodCrate.dds", nullptr, texture.ReleaseAndGetAddressOf()));
		Model barrelBaseModel{ device, Geometry::CreateBox(m_tankInfo.batteryWidth, m_tankInfo.batteryLength, m_tankInfo.batteryHeight) };
		barrelBaseModel.modelParts.front().texDiffuse = texture;

		GameObject& battery = m_battery;
		tank.AddChild(&battery);
		
		battery.SetModel(std::move(barrelBaseModel));
		battery.GetTransform().SetRotation(m_tankInfo.batteryRotation);
		battery.GetTransform().SetPosition(m_tankInfo.batteryPosition);
		
		// 炮管
		HR(CreateDDSTextureFromFile(device, L"Texture\\flare.dds", nullptr, texture.ReleaseAndGetAddressOf()));
		Model barrelModel{ device, Geometry::CreateCylinder(m_tankInfo.barrelCaliber, m_tankInfo.barrelLength, 20) };
		barrelModel.modelParts.front().texDiffuse = texture;

		GameObject& barrel = m_barrel;
		battery.AddChild(&barrel);
		
		barrel.SetModel(std::move(barrelModel));
		barrel.GetTransform().SetRotation(m_tankInfo.barrelRotation);
		barrel.GetTransform().SetPosition(m_tankInfo.barrelPosition);
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
			wheel0.GetTransform().SetRotation(m_tankInfo.wheelLeftRotation);
			wheel0.GetTransform().SetPosition(m_tankInfo.wheelLeftFrontPosition);
		}
		{
			// 右前轮
			GameObject& wheel1 = m_wheels[1];
			tank.AddChild(&wheel1);
			wheel1.SetModel(wheelModel);
			wheel1.GetTransform().SetRotation(m_tankInfo.wheelRightRotation);
			wheel1.GetTransform().SetPosition(m_tankInfo.wheelRightFrontPosition);
		}
		{
			// 左后轮
			GameObject& wheel2 = m_wheels[2];
			tank.AddChild(&wheel2);
			wheel2.SetModel(wheelModel);
			wheel2.GetTransform().SetRotation(m_tankInfo.wheelLeftRotation);
			wheel2.GetTransform().SetPosition(m_tankInfo.wheelLeftBackPosition);
		}
		{
			// 右后轮
			GameObject& wheel3 = m_wheels[3];
			tank.AddChild(&wheel3);
			wheel3.SetModel(wheelModel);
			wheel3.GetTransform().SetRotation(m_tankInfo.wheelRightRotation);
			wheel3.GetTransform().SetPosition(m_tankInfo.wheelRightBackPosition);
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
	Transform& tankTransform = m_tankMainBody.GetTransform();

	tankTransform.Translate(tankTransform.GetForwardAxisXM(), d);

	// 转轮子
	for(GameObject& wheel : m_wheels)
	{
		wheel.GetTransform().Rotate(XMVectorSet(3.14f * d, 0.0f, 0.0f, 0.0f));
	}
}

void Tank::Strafe(const float d)
{
	Transform& tankTransform = m_tankMainBody.GetTransform();
	// 转车身
	// @TODO 找一个比较合理的旋转速度
	tankTransform.Rotate(XMVectorSet(0.0f, 0.1f * d, 0.0f, 0.0f));
	const XMVECTOR upAxis = tankTransform.GetUpAxisXM();
	// 后轮不转
	// 轮胎绕Y轴旋转
	// @TODO 限制旋转范围
	m_wheels[0].GetTransform().RotateAround(XMLoadFloat3(&m_tankInfo.wheelLeftFrontPosition), upAxis, d);
	m_wheels[1].GetTransform().RotateAround(XMLoadFloat3(&m_tankInfo.wheelRightFrontPosition), upAxis, d);
}

Ray Tank::Shoot() const
{
	XMMATRIX barrelLocalToWorldMatrix = GetBarrelLocalToWorldMatrixXM();
	
	XMFLOAT3 look{};
	XMStoreFloat3(&look, barrelLocalToWorldMatrix.r[1]);
	XMFLOAT3 position{};
	XMStoreFloat3(&position, barrelLocalToWorldMatrix.r[3]);
	return { position,  look };
}

void Tank::Turn(const float d)
{
	Transform& batteryTransform = m_battery.GetTransform();
	// 转底座
	// @TODO 找一个比较合理的旋转速度
	batteryTransform.Rotate(XMVectorSet(0.0f, 0.25f * d, 0.0f, 0.0f));
}

XMFLOAT3 Tank::GetPosition() const
{
	return m_tankMainBody.GetTransform().GetPosition();
}

void Tank::AdjustPosition(FXMVECTOR minCoordinate, FXMVECTOR maxCoordinate)
{
	Transform& tankTransform = m_tankMainBody.GetTransform();

	XMFLOAT3 adjustedPos{};
	XMStoreFloat3(&adjustedPos, XMVectorClamp(tankTransform.GetPositionXM(), minCoordinate, maxCoordinate));
	tankTransform.SetPosition(adjustedPos);
}

void Tank::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	m_tankMainBody.Draw(deviceContext, effect);
}

XMMATRIX Tank::GetBarrelLocalToWorldMatrixXM() const
{
	const Transform& barrelTransform = m_barrel.GetTransform();
	const Transform& batteryTransform = m_battery.GetTransform();
	const Transform& tankTransform = m_tankMainBody.GetTransform();

	// 从最顶层子对象到最底层父对象运算: SSSSSSSS * R子T子 * RT * RT * RT * RT * RT * RT * R父T父
	
	return 
		XMMatrixScalingFromVector(barrelTransform.GetScaleXM()) *
		XMMatrixScalingFromVector(batteryTransform.GetScaleXM()) *
		XMMatrixScalingFromVector(tankTransform.GetScaleXM()) *

		XMMatrixRotationRollPitchYawFromVector(barrelTransform.GetRotationXM()) *
		XMMatrixTranslationFromVector(barrelTransform.GetPositionXM()) *

		XMMatrixRotationRollPitchYawFromVector(batteryTransform.GetRotationXM()) *
		XMMatrixTranslationFromVector(batteryTransform.GetPositionXM()) *

		XMMatrixRotationRollPitchYawFromVector(tankTransform.GetRotationXM()) *
		XMMatrixTranslationFromVector(tankTransform.GetPositionXM())
	;
}
