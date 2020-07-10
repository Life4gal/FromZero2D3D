#ifndef PLAYER_H
#define PLAYER_H

#include "DXTrace.h"
#include <array>

#include "GameObject.h"

class Player
{
	// 允许IMGUI面板自由访问数据
	friend class ImguiPanel;
	
public:
	struct VehicleInfo;
	
	Player(
		VehicleInfo tankInfo =
		{
			3.5f,
			6.0f,
			1.7f,
			2.0f,
			2.0f,
			1.2f,
			0.5f,
			4.5f,
			0.75f,
			0.5f
		}, 
		DirectX::XMFLOAT3 direction = {0.0f, 0.0f, 1.0f});

	void init(ID3D11Device* device);
	
	static Player& Get();
	
	void Walk(float d);
	void Strafe(float d);

	void AdjustPosition();

	// 获取物体变换
	Transform& GetTransform();
	// 获取物体变换
	const Transform& GetTransform() const;

	// 设置材质
	void SetMaterial(const Material& material);

	// 绘制
	void Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect);

	// 坦克相关信息的结果可以公开
	struct VehicleInfo
	{
		// ******************
		// 可变信息
		// @TODO

		// ******************
		// 不可变信息
		
		// 载具规格(立方体)
		const float m_bodyWidth;				// 载具俯视角(车头朝上下)宽度
		const float m_bodyLength;			// 载具俯视角(车头朝上下)长度
		const float m_bodyHeight;			// 载具高度
		// 炮台底座规格(立方体)
		const float m_barrelBaseWidth;		// 底座俯视角宽度
		const float m_barrelBaseLength;		// 底座俯视角长度
		const float m_barrelBaseHeight;		// 底座俯视角高度
		// 炮管规格(圆柱)
		const float m_barrelCaliber;			// 炮管的口径
		const float m_barrelLength;			// 炮管的长度
		// 轮子规格(圆柱)
		const float m_wheelRadius;			// 轮子的半径
		const float m_wheelLength;			// 轮子的长度
	};

private:
	// *************************
	// 以下结构定义为struct没有什么大问题,因为他们都是Player的私有结构,而且可以省去很多不必要操作
	
	struct Tank;
	// 轮子
	struct Wheel
	{
		enum class WheelPos
		{
			LeftFront,
			RightFront,
			LeftBack,
			RightBack
		};

		Wheel(WheelPos wheelPos);

		// 轮子限制在车上
		void AdjustPosition(const Tank& body, const VehicleInfo& tankInfo);

		// 前后移动
		void Walk(float d, const DirectX::XMFLOAT3& direction);
		// 左右转向,会改变方向
		void Strafe(float d, DirectX::XMFLOAT3& direction);

		// 自身对象
		GameObject m_self;

		// 轮子的相对车身位置,不可变
		const WheelPos m_wheelPos;
	};
	
	// 炮管底座
	struct BarrelBase
	{
		// 炮管底座限制在车上
		void AdjustPosition(const Tank& body, const VehicleInfo& tankInfo);

		struct Barrel
		{
			// 炮管限制在底座上
			void AdjustPosition(const BarrelBase& body, const VehicleInfo& tankInfo);

			// 自身对象
			GameObject m_self;
		};

		// 炮管
		Barrel m_barrel;
		// 自身对象
		GameObject m_self;
	};

	// 车身
	struct Tank
	{
		Tank(DirectX::XMFLOAT3 direction, VehicleInfo tankInfo);

		// 前后移动
		void Walk(float d);
		// 左右转向,会改变方向
		void Strafe(float d);
		
		// 限制车身移动范围
		void AdjustPosition();
		
		// 当前方向
		DirectX::XMFLOAT3 m_direction;
		// 四个轮子
		std::array<Wheel, 4> m_wheels;
		// 一个炮台
		BarrelBase m_barrelBase;
		// 自身对象
		GameObject m_self;
		
		// 坦克规格信息
		VehicleInfo m_tankInfo;
	};

	Tank m_tank;
};

#endif
