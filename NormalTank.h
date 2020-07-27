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
	
	// 载具车身(立方体)
	static constexpr float BodyWidth = 3.5f;		// 载具俯视角(车头朝上下)宽度
	static constexpr float BodyLength = 6.0f;		// 载具俯视角(车头朝上下)长度
	static constexpr float BodyHeight = 1.7f;		// 载具高度
	// 炮台底座规格(立方体)
	static constexpr float BatteryWidth = 2.0f;		// 底座俯视角宽度
	static constexpr float BatteryLength = 2.0f;	// 底座俯视角长度
	static constexpr float BatteryHeight = 1.2f;	// 底座俯视角高度
	// 炮管规格(圆柱)
	static constexpr float BarrelCaliber = 0.25;	// 炮管的口径
	static constexpr float BarrelLength = 4.5f;		// 炮管的长度
	// 轮子规格(圆柱)
	static constexpr float WheelRadius = 0.75f;		// 轮子的半径
	static constexpr float WheelLength = 0.5f;		// 轮子的长度

	struct Wheel
	{
		// 设置父子关系,必须在拷贝后再设置
		void SetParent()
		{
			wheel.AddChild(&wheelOutSide);
			wheel.AddChild(&wheelInSide);
		}
		
		GameObject wheel;							// 轮胎
		GameObject wheelOutSide;					// 外侧
		GameObject wheelInSide;						// 内侧
	};
	
	// 主体
	// 我们分成六个面以使用六张不同的纹理
	// 按顺序分别为 上/下/前/后/左/右
	std::array<GameObject, 6> m_tankMainBody;

	// 轮胎
	// 按顺序分别为 左前/右前/左后/右后
	std::array<Wheel, 4> m_wheels;
	
	// 炮台
	// 我们分成五个面以使用五张不同的纹理,因为下面总是不可见的
	// 按顺序分别为 上/前/后/左/右
	std::array<GameObject, 5> m_battery;
	
	// 炮管
	GameObject m_barrel;
};

#endif
