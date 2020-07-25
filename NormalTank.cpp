#include "NormalTank.h"

using namespace DirectX;

void NormalTank::Init(ID3D11Device* device)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;

	GameObject& body = m_tankMainBody[0];
	
	// 车身
	{
		// 上面为主体
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\body_top.dds", nullptr, texture.GetAddressOf()));
			Model model{ device, Geometry::CreatePlane(XMFLOAT2(3.5f, 6.0f), XMFLOAT2(1.0f, 1.0f)) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			body.SetModel(std::move(model));
			// 位置由之后设置
		}
		// 下面
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\body_bottom.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreatePlane(XMFLOAT2(3.5f, 6.0f), XMFLOAT2(1.0f, 1.0f)) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& bottom = m_tankMainBody[1];
			body.AddChild(&bottom);
			bottom.SetModel(std::move(model));
			
			BasicTransform& transform = bottom.GetTransform();
			// 绕那Z轴转180度
			transform.SetRotation(0.0f, 0.0f, XM_PI);
			// 矮车身的高度
			transform.SetPosition(0.0f, -1.7f, 0.0f);
		}
		// 前面
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\body_front.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreatePlane(XMFLOAT2(3.5f, 1.7f), XMFLOAT2(1.0f, 1.0f)) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& front = m_tankMainBody[2];
			body.AddChild(&front);
			front.SetModel(std::move(model));

			BasicTransform& transform = front.GetTransform();
			// 绕X轴转90度
			transform.SetRotation(XM_PIDIV2, 0.0f, 0.0f);
			// 矮半个车身的高度,偏半个车的长度
			transform.SetPosition(0.0f, -0.85f, 3.0f);
		}
		// 后面
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\body_back.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreatePlane(XMFLOAT2(3.5f, 1.7f), XMFLOAT2(1.0f, 1.0f)) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& back = m_tankMainBody[3];
			body.AddChild(&back);
			back.SetModel(std::move(model));

			BasicTransform& transform = back.GetTransform();
			// 绕X轴转-90度
			transform.SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
			// 矮半个车身的高度,偏半个车的长度
			transform.SetPosition(0.0f, -0.85f, -3.0f);
		}
		// 左面
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\body_left.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreatePlane(XMFLOAT2(1.7f, 6.0f), XMFLOAT2(1.0f, 1.0f)) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& left = m_tankMainBody[4];
			body.AddChild(&left);
			left.SetModel(std::move(model));

			BasicTransform& transform = left.GetTransform();
			// 绕Z轴转90度
			transform.SetRotation(0.0f, 0.0f, XM_PIDIV2);
			// 矮半个车身的高度,偏半个车的宽度
			transform.SetPosition(-1.75f, -0.85f, 0.0f);
		}
		// 右面
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\body_right.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreatePlane(XMFLOAT2(1.7f, 6.0f), XMFLOAT2(1.0f, 1.0f)) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& right = m_tankMainBody[5];
			body.AddChild(&right);
			right.SetModel(std::move(model));

			BasicTransform& transform = right.GetTransform();
			// 绕Z轴转-90度
			transform.SetRotation(0.0f, 0.0f, -XM_PIDIV2);
			// 矮半个车身的高度,偏半个车的宽度
			transform.SetPosition(1.75f, -0.85f, 0.0f);
		}
	}
	// 轮子
	{
		// 左前
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\wheel_leftfront.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreateCylinder(0.75f, 0.5f, 20) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& wheel = m_wheels[0];
			body.AddChild(&wheel);
			wheel.SetModel(std::move(model));

			BasicTransform& transform = wheel.GetTransform();
			// 绕Z轴转-90度
			transform.SetRotation(0.0f, 0.0f, -XM_PIDIV2);
			// 矮车身的高度,偏半个车的宽度,偏半个车的长度
			transform.SetPosition(-1.75f, -1.7f, 3.0f);
		}
		// 右前
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\wheel_rightfront.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreateCylinder(0.75f, 0.5f, 20) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& wheel = m_wheels[1];
			body.AddChild(&wheel);
			wheel.SetModel(std::move(model));

			BasicTransform& transform = wheel.GetTransform();
			// 绕Z轴转90度
			transform.SetRotation(0.0f, 0.0f, XM_PIDIV2);
			// 矮车身的高度,偏半个车的宽度,偏半个车的长度
			transform.SetPosition(1.75f, -1.7f, 3.0f);
		}
		// 左后
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\wheel_leftback.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreateCylinder(0.75f, 0.5f, 20) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& wheel = m_wheels[2];
			body.AddChild(&wheel);
			wheel.SetModel(std::move(model));

			BasicTransform& transform = wheel.GetTransform();
			// 绕Z轴转-90度
			transform.SetRotation(0.0f, 0.0f, -XM_PIDIV2);
			// 矮车身的高度,偏半个车的宽度,偏半个车的长度
			transform.SetPosition(-1.75f, -1.7f, -3.0f);
		}
		// 右后
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\wheel_rightback.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreateCylinder(0.75f, 0.5f, 20) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& wheel = m_wheels[3];
			body.AddChild(&wheel);
			wheel.SetModel(std::move(model));

			BasicTransform& transform = wheel.GetTransform();
			// 绕Z轴转90度
			transform.SetRotation(0.0f, 0.0f, XM_PIDIV2);
			// 矮车身的高度,偏半个车的宽度,偏半个车的长度
			transform.SetPosition(1.75f, -1.7f, -3.0f);
		}
	}
	// 炮台
	{
		GameObject& battery = m_battery[0];
		body.AddChild(&battery);
		// 上面为主体
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\battery_top.dds", nullptr, texture.GetAddressOf()));
			Model model{ device, Geometry::CreatePlane(XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f)) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			battery.SetModel(std::move(model));
			// 高底座的高度
			battery.GetTransform().SetPosition(0.0f, 1.2f, 0.0f);
		}
		// 前面
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\battery_front.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreatePlane(XMFLOAT2(2.0f, 1.2f), XMFLOAT2(1.0f, 1.0f)) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& front = m_battery[1];
			battery.AddChild(&front);
			front.SetModel(std::move(model));

			BasicTransform& transform = front.GetTransform();
			// 绕X轴转90度
			transform.SetRotation(XM_PIDIV2, 0.0f, 0.0f);
			// 矮半个底座的高度,偏半个底座的长度
			transform.SetPosition(0.0f, -0.6f, 1.0f);
		}
		// 后面
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\battery_back.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreatePlane(XMFLOAT2(2.0f, 1.2f), XMFLOAT2(1.0f, 1.0f)) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& back = m_battery[2];
			battery.AddChild(&back);
			back.SetModel(std::move(model));

			BasicTransform& transform = back.GetTransform();
			// 绕X轴转-90度
			transform.SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
			// 矮半个底座的高度,偏半个底座的长度
			transform.SetPosition(0.0f, -0.6f, -1.0f);
		}
		// 左面
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\battery_left.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreatePlane(XMFLOAT2(1.2f, 2.0f), XMFLOAT2(1.0f, 1.0f)) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& left = m_battery[3];
			battery.AddChild(&left);
			left.SetModel(std::move(model));

			BasicTransform& transform = left.GetTransform();
			// 绕Z轴转90度
			transform.SetRotation(0.0f, 0.0f, XM_PIDIV2);
			// 矮半个底座的高度,偏半个底座的宽度
			transform.SetPosition(-1.0f, -0.6f, 0.0f);
		}
		// 右面
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\battery_right.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreatePlane(XMFLOAT2(1.2f, 2.0f), XMFLOAT2(1.0f, 1.0f)) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& right = m_battery[4];
			battery.AddChild(&right);
			right.SetModel(std::move(model));

			BasicTransform& transform = right.GetTransform();
			// 绕Z轴转-90度
			transform.SetRotation(0.0f, 0.0f, -XM_PIDIV2);
			// 矮半个底座的高度,偏半个底座的宽度
			transform.SetPosition(1.0f, -0.6f, 0.0f);
		}
		// 炮管
		{
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\barrel.dds", nullptr, texture.ReleaseAndGetAddressOf()));
			Model model{ device, Geometry::CreateCylinder(0.25f, 4.5f, 20) };
			ModelPart& modelPart = model.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();
			modelPart.texDiffuse = texture;

			GameObject& barrel = m_barrel;
			battery.AddChild(&barrel);
			barrel.SetModel(std::move(model));

			BasicTransform& transform = barrel.GetTransform();
			// 绕X轴转90度
			transform.SetRotation(XM_PIDIV2, 0.0f, 0.0f);
			// 矮半个底座的高度,偏半个炮管的长度 + 半个底座的长度
			transform.SetPosition(0.0f, -0.6f, 3.25f);
		}
	}

	m_tankMainBody[0].SetDebugObjectName("TankBodyTop");
	m_tankMainBody[1].SetDebugObjectName("TankBodyBottom");
	m_tankMainBody[2].SetDebugObjectName("TankBodyFront");
	m_tankMainBody[3].SetDebugObjectName("TankBodyBack");
	m_tankMainBody[4].SetDebugObjectName("TankBodyLeft");
	m_tankMainBody[5].SetDebugObjectName("TankBodyRight");
	m_wheels[0].SetDebugObjectName("TankWheelLeftFront");
	m_wheels[1].SetDebugObjectName("TankWheelRightFront");
	m_wheels[2].SetDebugObjectName("TankWheelLeftBack");
	m_wheels[3].SetDebugObjectName("TankWheelRightBack");
	m_battery[0].SetDebugObjectName("TankBatteryTop");
	m_battery[1].SetDebugObjectName("TankBatteryFront");
	m_battery[2].SetDebugObjectName("TankBatteryBack");
	m_battery[3].SetDebugObjectName("TankBatteryLeft");
	m_battery[4].SetDebugObjectName("TankBatteryRight");
	m_barrel.SetDebugObjectName("TankBarrel");
}

void NormalTank::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	m_tankMainBody[0].Draw(deviceContext, effect);
}

BasicTransform& NormalTank::GetTankTransform()
{
	return m_tankMainBody[0].GetTransform();
}

const BasicTransform& NormalTank::GetTankTransform() const
{
	return m_tankMainBody[0].GetTransform();
}

XMFLOAT3 NormalTank::GetTankPosition() const
{
	// 我们是以车身上面为主体,所以我们实际的Y轴位置要减去半个车身高度
	const XMFLOAT3 position = m_tankMainBody[0].GetTransform().GetPositionFloat3();

	return { position.x, position.y - 0.85f, position.z };
}

void NormalTank::SetTankPosition(const XMFLOAT3& position)
{
	// 我们是以车身上面为主体,所以我们实际的Y轴位置要加去半个车身高度
	GetTankTransform().SetPosition(position.x, position.y + 0.85f, position.z);
}

void NormalTank::RotateWheels(const float d)
{
	for (GameObject& wheel : m_wheels)
	{
		wheel.GetTransform().Rotate(XMVectorSet(3.14f * d, 0.0f, 0.0f, 0.0f));
	}
}

XMMATRIX NormalTank::GetBarrelLocalToWorldMatrix() const
{
	const BasicTransform& barrelTransform = m_barrel.GetTransform();
	const BasicTransform& batteryTransform = m_battery[0].GetTransform();
	const BasicTransform& tankTransform = m_tankMainBody[0].GetTransform();

	// 从最顶层子对象到最底层父对象运算: SSSSSSSS * R子T子 * RT * RT * RT * RT * RT * RT * R父T父

	return
		barrelTransform.GetScaleMatrix() *
		batteryTransform.GetScaleMatrix() *
		tankTransform.GetScaleMatrix() *

		barrelTransform.GetRotationTranslationMatrix() *
		batteryTransform.GetRotationTranslationMatrix() *
		tankTransform.GetRotationTranslationMatrix();
	
}

BasicTransform& NormalTank::GetBatteryTransform()
{
	return m_battery[0].GetTransform();
}

const BasicTransform& NormalTank::GetBatteryTransform() const
{
	return m_battery[0].GetTransform();
}
