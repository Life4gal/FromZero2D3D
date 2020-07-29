#include "NormalTank.h"

using namespace DirectX;

void NormalTank::Init(ID3D11Device* device)
{
	GameObject& body = m_tankMainBody[0];

	// 车身
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bodyTex;
		HR(CreateWICTextureFromFile(device, L"Texture\\Tank\\pic.jpg", nullptr, bodyTex.GetAddressOf()));

		// 上面为主体
		{
			// TODO 我们需要重新设计这一部分以减少因仅仅修改大小尺寸而导致完全重新初始化带来的代价
			Model tankModel{ device, Geometry::CreatePlane(XMFLOAT2(BodyWidth, BodyLength), XMFLOAT2(1.0f, 1.0f)) };
			{
				ModelPart& modelPart = tankModel.modelParts.front();
				modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
				modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
				modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
				modelPart.material.reflect = XMFLOAT4();
				modelPart.texDiffuse = bodyTex;
			}

			body.SetModel(std::move(tankModel));
			// 位置由之后设置
		}
		// 下面
		{
			Model tankModel{ device, Geometry::CreatePlane(XMFLOAT2(BodyWidth, BodyLength), XMFLOAT2(1.0f, 1.0f)) };
			{
				ModelPart& modelPart = tankModel.modelParts.front();
				modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
				modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
				modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
				modelPart.material.reflect = XMFLOAT4();
				modelPart.texDiffuse = bodyTex;
			}

			GameObject& bottom = m_tankMainBody[1];
			body.AddChild(&bottom);
			bottom.SetModel(std::move(tankModel));
			
			BasicTransform& transform = bottom.GetTransform();
			// 绕那Z轴转180度
			transform.SetRotation(0.0f, 0.0f, XM_PI);
			// 矮车身的高度
			transform.SetPosition(0.0f, -BodyHeight, 0.0f);
		}
		// 前面
		{
			Model tankModel{ device, Geometry::CreatePlane(XMFLOAT2(BodyWidth, BodyHeight), XMFLOAT2(1.0f, 1.0f)) };
			{
				ModelPart& modelPart = tankModel.modelParts.front();
				modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
				modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
				modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
				modelPart.material.reflect = XMFLOAT4();
				modelPart.texDiffuse = bodyTex;
			}
			
			GameObject& front = m_tankMainBody[2];
			body.AddChild(&front);
			front.SetModel(std::move(tankModel));

			BasicTransform& transform = front.GetTransform();
			// 绕X轴转-90度,绕Z轴180度
			transform.SetRotation(-XM_PIDIV2, 0.0f, XM_PI);
			// 矮半个车身的高度,偏半个车的长度
			transform.SetPosition(0.0f, -BodyHeight / 2, BodyLength / 2);
		}
		// 后面
		{
			Model tankModel{ device, Geometry::CreatePlane(XMFLOAT2(BodyWidth, BodyHeight), XMFLOAT2(1.0f, 1.0f)) };
			{
				ModelPart& modelPart = tankModel.modelParts.front();
				modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
				modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
				modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
				modelPart.material.reflect = XMFLOAT4();
				modelPart.texDiffuse = bodyTex;
			}

			GameObject& back = m_tankMainBody[3];
			body.AddChild(&back);
			back.SetModel(std::move(tankModel));

			BasicTransform& transform = back.GetTransform();
			// 绕X轴转-90度
			transform.SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
			// 矮半个车身的高度,偏半个车的长度
			transform.SetPosition(0.0f, -BodyHeight / 2, -BodyLength / 2);
		}
		// 左面
		{
			Model tankModel{ device, Geometry::CreatePlane(XMFLOAT2(BodyLength, BodyHeight), XMFLOAT2(1.0f, 1.0f)) };
			{
				ModelPart& modelPart = tankModel.modelParts.front();
				modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
				modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
				modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
				modelPart.material.reflect = XMFLOAT4();
				modelPart.texDiffuse = bodyTex;
			}

			GameObject& left = m_tankMainBody[4];
			body.AddChild(&left);
			left.SetModel(std::move(tankModel));

			BasicTransform& transform = left.GetTransform();
			// 绕Z轴转90度,绕X轴转-90度
			transform.SetRotation(-XM_PIDIV2, 0.0f, XM_PIDIV2);
			// 矮半个车身的高度,偏半个车的宽度
			transform.SetPosition(-BodyWidth / 2, -BodyHeight / 2, 0.0f);
		}
		// 右面
		{
			Model tankModel{ device, Geometry::CreatePlane(XMFLOAT2(BodyLength, BodyHeight), XMFLOAT2(1.0f, 1.0f)) };
			{
				ModelPart& modelPart = tankModel.modelParts.front();
				modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
				modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
				modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
				modelPart.material.reflect = XMFLOAT4();
				modelPart.texDiffuse = bodyTex;
			}

			GameObject& right = m_tankMainBody[5];
			body.AddChild(&right);
			right.SetModel(std::move(tankModel));

			BasicTransform& transform = right.GetTransform();
			// 绕Z轴转-90度,绕X轴转-90度
			transform.SetRotation(-XM_PIDIV2, 0.0f, -XM_PIDIV2);
			// 矮半个车身的高度,偏半个车的宽度
			transform.SetPosition(BodyWidth / 2, -BodyHeight / 2, 0.0f);
		}
	}
	// 轮子
	{
		Wheel wheel;
		{
			{
				Model model{ device, Geometry::CreateCylinderNoCap(WheelRadius, WheelLength) };
				ModelPart& modelPart = model.modelParts.front();
				modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
				modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
				modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
				modelPart.material.reflect = XMFLOAT4();

				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> wheelTex;
				HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\wheel.dds", nullptr, wheelTex.GetAddressOf()));
				modelPart.texDiffuse = wheelTex;

				wheel.wheel.SetModel(std::move(model));
			}
			{
				Model model{ device, Geometry::CreateCircle(WheelRadius) };
				ModelPart& modelPart = model.modelParts.front();
				modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
				modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
				modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
				modelPart.material.reflect = XMFLOAT4();

				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> wheelSideTex;
				HR(CreateWICTextureFromFile(device, L"Texture\\Tank\\pic.jpg", nullptr, wheelSideTex.GetAddressOf()));
				modelPart.texDiffuse = wheelSideTex;

				wheel.wheelOutSide.SetModel(model);
				wheel.wheelInSide.SetModel(std::move(model));
			}
		}
		// 左前
		{
			Wheel& wheel0 = m_wheels[0];
			wheel0 = wheel;
			body.AddChild(&wheel0.wheel);
			// 车胎
			{			
				BasicTransform& transform = wheel0.wheel.GetTransform();
				// 绕Z轴转90度
				transform.SetRotation(0.0f, 0.0f, XM_PIDIV2);
				// 矮车身的高度,偏半个车的宽度,偏半个车的长度
				transform.SetPosition(-BodyWidth / 2, -BodyHeight, BodyLength / 2);
			}
			// 车轮侧面
			{
				//左前外
				{
					BasicTransform& transform = wheel0.wheelOutSide.GetTransform();
					// 偏半个轮子宽度
					transform.SetPosition(0.0f, WheelLength / 2, 0.0f);
					// 绕Y轴转90度
					transform.SetRotation(0.0f, XM_PIDIV2, 0.0f);
				}
				// 左前内
				{
					BasicTransform& transform = wheel0.wheelInSide.GetTransform();
					// 偏半个轮子宽度
					transform.SetPosition(0.0f, -WheelLength / 2, 0.0f);
					// 绕Z轴转180度,绕Y轴转-90度
					transform.SetRotation(0.0f, XM_PIDIV2, XM_PI);
				}
			}
		}
		// 右前
		{
			Wheel& wheel1 = m_wheels[1];
			wheel1 = wheel;
			body.AddChild(&wheel1.wheel);
			// 车胎
			{
				BasicTransform& transform = wheel1.wheel.GetTransform();
				// 绕Z轴转-90度
				transform.SetRotation(0.0f, 0.0f, -XM_PIDIV2);
				// 矮车身的高度,偏半个车的宽度,偏半个车的长度
				transform.SetPosition(BodyWidth / 2, -BodyHeight, BodyLength / 2);
			}
			// 车轮侧面
			{
				//右前外
				{
					BasicTransform& transform = wheel1.wheelOutSide.GetTransform();
					// 偏半个轮子宽度
					transform.SetPosition(0.0f, WheelLength / 2, 0.0f);
					// 绕Y轴转-90度
					transform.SetRotation(0.0f, -XM_PIDIV2, 0.0f);
				}
				// 右前内
				{
					BasicTransform& transform = wheel1.wheelInSide.GetTransform();
					// 偏半个轮子宽度
					transform.SetPosition(0.0f, -WheelLength / 2, 0.0f);
					// 绕Z轴转180度,绕Y轴转-90度
					transform.SetRotation(0.0f, -XM_PIDIV2, XM_PI);
				}
			}
		}
		// 左后
		{
			Wheel& wheel2 = m_wheels[2];
			wheel2 = wheel;
			body.AddChild(&wheel2.wheel);
			{
				BasicTransform& transform = wheel2.wheel.GetTransform();
				// 绕Z轴转-90度
				transform.SetRotation(0.0f, 0.0f, XM_PIDIV2);
				// 矮车身的高度,偏半个车的宽度,偏半个车的长度
				transform.SetPosition(-BodyWidth / 2, -BodyHeight, -BodyLength / 2);
			}
			// 车轮侧面
			{
				//左后外
				{
					BasicTransform& transform = wheel2.wheelOutSide.GetTransform();
					// 偏半个轮子宽度
					transform.SetPosition(0.0f, WheelLength / 2, 0.0f);
					// 绕Y轴转90度
					transform.SetRotation(0.0f, XM_PIDIV2, 0.0f);
				}
				// 左后内
				{
					BasicTransform& transform = wheel2.wheelInSide.GetTransform();
					// 偏半个轮子宽度
					transform.SetPosition(0.0f, -WheelLength / 2, 0.0f);
					// 绕Z轴转180度,绕Y轴转90度
					transform.SetRotation(0.0f, XM_PIDIV2, XM_PI);
				}
			}
		}
		// 右后
		{
			Wheel& wheel3 = m_wheels[3];
			wheel3 = wheel;
			body.AddChild(&wheel3.wheel);
			{
				BasicTransform& transform = wheel3.wheel.GetTransform();
				// 绕Z轴转90度
				transform.SetRotation(0.0f, 0.0f, -XM_PIDIV2);
				// 矮车身的高度,偏半个车的宽度,偏半个车的长度
				transform.SetPosition(BodyWidth / 2, -BodyHeight, -BodyLength / 2);
			}
			// 车轮侧面
			{
				//右后外
				{
					BasicTransform& transform = wheel3.wheelOutSide.GetTransform();
					// 偏半个轮子宽度
					transform.SetPosition(0.0f, WheelLength / 2, 0.0f);
					// 绕Y轴转-90度
					transform.SetRotation(0.0f, -XM_PIDIV2, 0.0f);
				}
				// 右后内
				{
					BasicTransform& transform = wheel3.wheelInSide.GetTransform();
					// 偏半个轮子宽度
					transform.SetPosition(0.0f, -WheelLength / 2, 0.0f);
					// 绕Z轴转180度,绕Y轴转-90度
					transform.SetRotation(0.0f, -XM_PIDIV2, XM_PI);
				}
			}
		}
	}
	// 炮台
	{
		
		GameObject& battery = m_battery[0];
		body.AddChild(&battery);

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
		HR(CreateWICTextureFromFile(device, L"Texture\\Tank\\pic.jpg", nullptr, texture.GetAddressOf()));
		
		// 上面为主体
		{
			Model batteryModel{ device, Geometry::CreatePlane(XMFLOAT2(BatteryWidth, BatteryLength), XMFLOAT2(1.0f, 1.0f)) };
			{
				ModelPart& modelPart = batteryModel.modelParts.front();
				modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
				modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
				modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
				modelPart.material.reflect = XMFLOAT4();
				modelPart.texDiffuse = texture;
			}

			battery.SetModel(std::move(batteryModel));
			// 高底座的高度
			battery.GetTransform().SetPosition(0.0f, BatteryHeight, 0.0f);
		}
		// 前面
		{
			Model batteryModel{ device, Geometry::CreatePlane(XMFLOAT2(BatteryWidth, BatteryHeight), XMFLOAT2(1.0f, 1.0f)) };
			{
				ModelPart& modelPart = batteryModel.modelParts.front();
				modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
				modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
				modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
				modelPart.material.reflect = XMFLOAT4();
				modelPart.texDiffuse = texture;
			}

			GameObject& front = m_battery[1];
			battery.AddChild(&front);
			front.SetModel(std::move(batteryModel));

			BasicTransform& transform = front.GetTransform();
			// 绕X轴转-90度,绕Z轴180度
			transform.SetRotation(-XM_PIDIV2, 0.0f, XM_PI);
			// 矮半个底座的高度,偏半个底座的长度
			transform.SetPosition(0.0f, -BatteryHeight / 2, BatteryLength / 2);
		}
		// 后面
		{
			Model batteryModel{ device, Geometry::CreatePlane(XMFLOAT2(BatteryWidth, BatteryHeight), XMFLOAT2(1.0f, 1.0f)) };
			{
				ModelPart& modelPart = batteryModel.modelParts.front();
				modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
				modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
				modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
				modelPart.material.reflect = XMFLOAT4();
				modelPart.texDiffuse = texture;
			}

			GameObject& back = m_battery[2];
			battery.AddChild(&back);
			back.SetModel(std::move(batteryModel));

			BasicTransform& transform = back.GetTransform();
			// 绕X轴转-90度
			transform.SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
			// 矮半个底座的高度,偏半个底座的长度
			transform.SetPosition(0.0f, -BatteryHeight / 2, -BatteryLength / 2);
		}
		// 左面
		{
			Model batteryModel{ device, Geometry::CreatePlane(XMFLOAT2(BatteryLength, BatteryHeight), XMFLOAT2(1.0f, 1.0f)) };
			{
				ModelPart& modelPart = batteryModel.modelParts.front();
				modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
				modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
				modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
				modelPart.material.reflect = XMFLOAT4();
				modelPart.texDiffuse = texture;
			}

			GameObject& left = m_battery[3];
			battery.AddChild(&left);
			left.SetModel(std::move(batteryModel));

			BasicTransform& transform = left.GetTransform();
			// 绕Z轴转90度,绕X轴转-90度
			transform.SetRotation(-XM_PIDIV2, 0.0f, XM_PIDIV2);
			// 矮半个底座的高度,偏半个底座的宽度
			transform.SetPosition(-BatteryWidth / 2, -BatteryHeight / 2, 0.0f);
		}
		// 右面
		{
			Model batteryModel{ device, Geometry::CreatePlane(XMFLOAT2(BatteryLength, BatteryHeight), XMFLOAT2(1.0f, 1.0f)) };
			{
				ModelPart& modelPart = batteryModel.modelParts.front();
				modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
				modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
				modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
				modelPart.material.reflect = XMFLOAT4();
				modelPart.texDiffuse = texture;
			}

			GameObject& right = m_battery[4];
			battery.AddChild(&right);
			right.SetModel(std::move(batteryModel));

			BasicTransform& transform = right.GetTransform();
			// 绕Z轴转-90度,绕X轴转-90度
			transform.SetRotation(-XM_PIDIV2, 0.0f, -XM_PIDIV2);
			// 矮半个底座的高度,偏半个底座的宽度
			transform.SetPosition(BatteryWidth / 2, -BatteryHeight / 2, 0.0f);
		}
		// 炮管
		{
			Model barrelModel{ device, Geometry::CreateCylinder(BarrelCaliber, BarrelLength) };
			ModelPart& modelPart = barrelModel.modelParts.front();
			modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
			modelPart.material.reflect = XMFLOAT4();

			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> barrelTex;
			HR(CreateDDSTextureFromFile(device, L"Texture\\Tank\\barrel.dds", nullptr, barrelTex.ReleaseAndGetAddressOf()));
			modelPart.texDiffuse = barrelTex;

			GameObject& barrel = m_barrel;
			battery.AddChild(&barrel);
			barrel.SetModel(std::move(barrelModel));

			BasicTransform& transform = barrel.GetTransform();
			// 绕X轴转90度
			transform.SetRotation(XM_PIDIV2, 0.0f, 0.0f);
			// 矮半个底座的高度,偏半个炮管的长度 + 半个底座的长度
			transform.SetPosition(0.0f, -BatteryHeight / 2, BarrelLength / 2 + BatteryLength / 2);
		}
	}

	m_tankMainBody[0].SetDebugObjectName("TankBodyTop");
	m_tankMainBody[1].SetDebugObjectName("TankBodyBottom");
	m_tankMainBody[2].SetDebugObjectName("TankBodyFront");
	m_tankMainBody[3].SetDebugObjectName("TankBodyBack");
	m_tankMainBody[4].SetDebugObjectName("TankBodyLeft");
	m_tankMainBody[5].SetDebugObjectName("TankBodyRight");

	m_wheels[0].wheel.SetDebugObjectName("TankWheel0_wheel");
	m_wheels[0].wheelOutSide.SetDebugObjectName("TankWheel0_wheelOutSide");
	m_wheels[0].wheelInSide.SetDebugObjectName("TankWheel0_wheelInSide");
	m_wheels[1].wheel.SetDebugObjectName("TankWheel1_wheel");
	m_wheels[1].wheelOutSide.SetDebugObjectName("TankWheel1_wheelOutSide");
	m_wheels[1].wheelInSide.SetDebugObjectName("TankWheel1_wheelInSide");
	m_wheels[2].wheel.SetDebugObjectName("TankWheel2_wheel");
	m_wheels[2].wheelOutSide.SetDebugObjectName("TankWheel2_wheelOutSide");
	m_wheels[2].wheelInSide.SetDebugObjectName("TankWheel2_wheelInSide");
	m_wheels[3].wheel.SetDebugObjectName("TankWheel3_wheel");
	m_wheels[3].wheelOutSide.SetDebugObjectName("TankWheel3_wheelOutSide");
	m_wheels[3].wheelInSide.SetDebugObjectName("TankWheel3_wheelInSide");
	
	m_battery[0].SetDebugObjectName("TankBatteryTop");
	m_battery[1].SetDebugObjectName("TankBatteryFront");
	m_battery[2].SetDebugObjectName("TankBatteryBack");
	m_battery[3].SetDebugObjectName("TankBatteryLeft");
	m_battery[4].SetDebugObjectName("TankBatteryRight");
	
	m_barrel.SetDebugObjectName("TankBarrel");
}

void NormalTank::Draw(ID3D11DeviceContext* deviceContext, IEffect* effect)
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
	for (Wheel& wheel : m_wheels)
	{
		wheel.wheel.GetTransform().Rotate(XMVectorSet(3.14f * d, 0.0f, 0.0f, 0.0f));
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
