#ifndef NORMALTANK_H
#define NORMALTANK_H

#include "TankStructureHelper.h"

class NormalTank final : public BasicTankStructure
{
	friend class ImguiPanel;
public:
	NormalTank() = default;
	~NormalTank() override = default;

	NormalTank(const NormalTank& other) = default;
	NormalTank(NormalTank&& other) noexcept = default;
	NormalTank& operator=(const NormalTank& other) = default;
	NormalTank& operator=(NormalTank&& other) noexcept = default;
	
	void Init(ID3D11Device* device) override;

	void Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect) override;
	
private:
	BasicTransform& GetTankTransform() override;

	const BasicTransform& GetTankTransform() const override;

	DirectX::XMFLOAT3 GetTankPosition() const override;
	
	void SetTankPosition(const DirectX::XMFLOAT3& position) override;
	
	void RotateWheels(float d) override;
	
	DirectX::XMMATRIX GetBarrelLocalToWorldMatrix() const override;
	
	BasicTransform& GetBatteryTransform() override;

	const BasicTransform& GetBatteryTransform() const override;
	
	/*
		基本信息:
			// 载具车身(立方体)
			const float bodyWidth = 3.5f;			// 载具俯视角(车头朝上下)宽度
			const float bodyLength = 6.0f;			// 载具俯视角(车头朝上下)长度
			const float bodyHeight = 1.7f;			// 载具高度

			// 炮台底座规格(立方体)
			const float batteryWidth = 2.0f;		// 底座俯视角宽度
			const float batteryLength = 2.0f;		// 底座俯视角长度
			const float batteryHeight = 1.2f;		// 底座俯视角高度
			
			// 炮管规格(圆柱)
			const float barrelCaliber = 0.25f;		// 炮管的口径
			const float barrelLength = 4.5f;		// 炮管的长度
			
			// 轮子规格(圆柱)
			const float wheelRadius = 0.75f;		// 轮子的半径
			const float wheelLength = 0.5f;			// 轮子的长度
	*/
	
	// 主体
	// 我们分成六个面以使用六张不同的纹理
	// 按顺序分别为 上/下/前/后/左/右
	std::array<GameObject, 6> m_tankMainBody;

	// 轮胎
	// 按顺序分别为 左前/右前/左后/右后
	std::array<GameObject, 4> m_wheels;
	// 我们把车轱辘和车胎分开了
	// 轮胎侧面
	// 按顺序分别为 左前外/左前内/右前外/右前内/左后外/左后内/右后外/右后内
	std::array<GameObject, 8> m_wheelsSide;
	
	// 炮台
	// 我们分成五个面以使用五张不同的纹理,因为下面总是不可见的
	// 按顺序分别为 上/前/后/左/右
	std::array<GameObject, 5> m_battery;
	
	// 炮管
	GameObject m_barrel;
};

#endif
