#ifndef PLAYER_H
#define PLAYER_H

#include "GameObject.h"
#include "DXTrace.h"
#include <array>

class Player
{
	friend class GameApp;
public:
	// bodyHeight 车顶至车底的距离
	// bodyLength 车头至车尾的距离
	// bodyWidth 车左侧至右侧的距离
	Player(DirectX::XMFLOAT3 direction = {0.0f, 0.0f, 1.0f}, float bodyWidth = 3.5f, float bodyLength = 6.0f, float bodyHeight = 1.7f);

	void init(ID3D11Device* device);
	
	static Player& Get();

	void SetDirection(const DirectX::XMFLOAT3& direction);
	const DirectX::XMFLOAT3& GetDirection() const;
	
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

private:
	class VehiclePart
	{
	public:
		VehiclePart() = default;
		virtual ~VehiclePart() = default;

		VehiclePart(const VehiclePart& other) = default;
		VehiclePart(VehiclePart&& other) noexcept = default;
		VehiclePart& operator=(const VehiclePart& other) = default;
		VehiclePart& operator=(VehiclePart&& other) noexcept = default;
		
		// 获取组件
		GameObject& GetPartObject();
		// 获取组件
		const GameObject& GetPartObject() const;

	protected:
		GameObject m_part;
	};

	class Car;
	// 轮子
	class Wheel : public VehiclePart
	{
		// 只允许车身控制轮子
		friend class Car;
	public:
		enum class WheelPos
		{
			LeftFront,
			RightFront,
			LeftBack,
			RightBack
		};

		Wheel(WheelPos wheelPos);

	private:
		// 轮子限制在车上,需要车子的中心点和车长/宽/高和轮子所属位置
		void AdjustPosition(Car& body);

		// 前后移动
		void Walk(float d, const DirectX::XMFLOAT3& direction);
		// 左右转向,会改变方向
		void Strafe(float d, DirectX::XMFLOAT3& direction);

	public:
		// 轮子的相对车身位置,不可变
		const WheelPos m_wheelPos;
	};

	// 车身
	class Car : public VehiclePart
	{
		// 允许轮子自由存取车身数据,可以省去很多参数传递
		friend class Wheel;
		// 允许这三个函数访问轮子
		friend void Player::SetMaterial(const Material& material);
		friend void Player::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect);
		friend void Player::init(ID3D11Device* device);
		
	public:
		Car(DirectX::XMFLOAT3 direction, float bodyWidth, float bodyLength, float bodyHeight);

		void SetDirection(const DirectX::XMFLOAT3& direction);
		const DirectX::XMFLOAT3& GetDirection() const;
		
		// 前后移动
		void Walk(float d);
		// 左右转向,会改变方向
		void Strafe(float d);
		
		// 限制车身移动范围
		void AdjustPosition();
		
	private:
		// 当前方向
		DirectX::XMFLOAT3 m_direction;
		// 四个轮子
		std::array<Wheel, 4> m_wheels;

	public:
		// 车子长宽高,不可变
		const float m_bodyWidth;
		const float m_bodyLength;
		const float m_bodyHeight;
	};

	Car m_car;
};

#endif
