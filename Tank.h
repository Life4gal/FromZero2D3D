#ifndef TANK_H
#define TANK_H

#include "GameObject.h"
#include "Collision.h"

#include <array>

class Tank
{
	// 允许IMGUI面板自由访问数据
	friend class ImguiPanel;
	
public:
	struct VehicleInfo;
	
	explicit Tank(
		VehicleInfo tankInfo = NormalTank
	);
	
	void Init(
		ID3D11Device* device,
		BasicTransform&& transform = BasicTransform
		{
			{1.0f, 1.0f, 1.0f},
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.5f, -20.0f}
		}
	);

	void Walk(float d);
	void Strafe(float d);
	Ray Shoot() const;
	// 转动炮管,大于0向右转
	void Turn(float d);

	DirectX::XMFLOAT3 GetPosition() const;

	void AdjustPosition(
		DirectX::FXMVECTOR minCoordinate = { { -25.0f, 0.5f, -25.0f, 0.0f } },
		DirectX::FXMVECTOR maxCoordinate = { { 25.0f, 0.5f , 25.0f, 0.0f } });
	
	// 绘制
	void Draw(ID3D11DeviceContext* deviceContext, IEffect* effect);
	
	// 坦克相关信息的结果可以公开
	struct VehicleInfo
	{
		// ******************
		// 不可变信息

		// 载具车身(立方体)
		const float bodyWidth;			// 载具俯视角(车头朝上下)宽度
		const float bodyLength;			// 载具俯视角(车头朝上下)长度
		const float bodyHeight;			// 载具高度
		// 炮台底座规格(立方体)
		const float batteryWidth;		// 底座俯视角宽度
		const float batteryLength;		// 底座俯视角长度
		const float batteryHeight;		// 底座俯视角高度
		const DirectX::XMFLOAT3 batteryScale;		// 底座相对车身的缩放
		const DirectX::XMFLOAT3 batteryRotation;	// 底座相对车身的旋转
		const DirectX::XMFLOAT3 batteryPosition;	// 底座相对车身的位置
		
		// 炮管规格(圆柱)
		const float barrelCaliber;			// 炮管的口径
		const float barrelLength;			// 炮管的长度
		const DirectX::XMFLOAT3 barrelScale;	// 炮管相对底座的缩放
		const DirectX::XMFLOAT3 barrelRotation;	// 炮管相对底座的旋转
		const DirectX::XMFLOAT3 barrelPosition;	// 炮管相对底座的位置
		
		// 轮子规格(圆柱)
		const float wheelRadius;			// 轮子的半径
		const float wheelLength;			// 轮子的长度
		const DirectX::XMFLOAT3 wheelScale;			// 轮子相对车身的缩放,我们暂时认为轮子的缩放是一样的
		const DirectX::XMFLOAT3 wheelLeftRotation;	// 轮子相对车身的旋转,我们暂时认为同一边的轮子的旋转是一样的
		const DirectX::XMFLOAT3 wheelRightRotation;	// 分两边是为了可以使用同一个贴图
		const DirectX::XMFLOAT3 wheelLeftFrontPosition;		// 左前轮相对车身的位置
		const DirectX::XMFLOAT3 wheelRightFrontPosition;	// 右前轮相对车身的位置
		const DirectX::XMFLOAT3 wheelLeftBackPosition;		// 左后轮相对车身的位置
		const DirectX::XMFLOAT3 wheelRightBackPosition;		// 右后轮相对车身的位置
		
		// ******************
		// 可变信息
	};

	static const VehicleInfo NormalTank;
	
private:
	// 采用相对位置需要自己算绝对位置
	// 炮管
	DirectX::XMMATRIX GetBarrelLocalToWorldMatrix() const;
	// 底座和轮子只需要知道车身位置就很容易算出
	
	GameObject m_tankMainBody;

	// 轮胎,分别为: 左前,右前,左后,右后
	std::array<GameObject, 4> m_wheels;
	// 炮台
	GameObject m_battery;
	// 炮管
	GameObject m_barrel;

	VehicleInfo m_tankInfo;
};

#endif
