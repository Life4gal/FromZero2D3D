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
		Transform transform =
		{
			{1.0f, 1.0f, 1.0f},
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.5f, 0.0f}
		}
	);

	void Walk(float d);
	void Strafe(float d);
	Ray Shoot();
	// 转动炮管,大于0向右转
	void Turn(float d);

	DirectX::XMFLOAT3 GetPosition() const;

	void AdjustPosition(
		DirectX::FXMVECTOR minCoordinate = { { -25.0f, 0.5f, -25.0f, 0.0f } },
		DirectX::FXMVECTOR maxCoordinate = { { 25.0f, 0.5f , 25.0f, 0.0f } });
	
	// 绘制
	void Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect);
	
	// 坦克相关信息的结果可以公开
	struct VehicleInfo
	{
		// ******************
		// 不可变信息

		// 载具规格(立方体)
		const float bodyWidth;				// 载具俯视角(车头朝上下)宽度
		const float bodyLength;			// 载具俯视角(车头朝上下)长度
		const float bodyHeight;			// 载具高度
		// 炮台底座规格(立方体)
		const float barrelBaseWidth;		// 底座俯视角宽度
		const float barrelBaseLength;		// 底座俯视角长度
		const float barrelBaseHeight;		// 底座俯视角高度
		// 炮管规格(圆柱)
		const float barrelCaliber;			// 炮管的口径
		const float barrelLength;			// 炮管的长度
		// 轮子规格(圆柱)
		const float wheelRadius;			// 轮子的半径
		const float wheelLength;			// 轮子的长度

		// ******************
		// 可变信息
	};

	static const VehicleInfo NormalTank;
	
private:
	// 采用相对位置需要自己算绝对位置
	// 炮管
	DirectX::XMFLOAT3 GetBarrelWorldPosition() const;
	// 底座和轮子只需要知道车身位置就很容易算出
	
	GameObject m_tankRoot;

	// 轮胎,分别为: 左前,右前,左后,右后
	std::array<GameObject, 4> m_wheels;
	// 炮台
	GameObject m_battery;
	// 炮管
	GameObject m_barrel;

	VehicleInfo m_tankInfo;
};

#endif
