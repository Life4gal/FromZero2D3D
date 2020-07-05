#ifndef PLAYER_H
#define PLAYER_H

#include "GameObject.h"
#include "DXTrace.h"
#include <array>

class Player
{
public:
	Player();

	/**
	 * \brief 
	 * \param device 
	 * \param bodyHeight 车顶至车底的距离
	 * \param bodyLength 车头至车尾的距离
	 * \param bodyWidth 车左侧至右侧的距离
	 */
	void init(ID3D11Device* device, float bodyHeight = 1.7f, float bodyLength = 6.0f, float bodyWidth = 3.5f);
	
	static Player& Get();
	
	void Walk(float d);
	void Strafe(float d);
	void Jump(float d);

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
	// 我们不需要额外的数据成员,所以尽管 GameObject 没有一个虚析构也不会对我们产生什么影响
	class VehiclePart : public GameObject
	{
	public:
		
		// 直行(平面移动)
		virtual void Walk(float d);
		// 平移
		virtual void Strafe(float d);
	};

	// 轮子
	class Wheel : public VehiclePart 
	{
	public:
		enum class WheelPos
		{
			LeftFront,
			RightFront,
			LeftBack,
			RightBack
		};
		// 轮子还需要滚起来
		void Walk(float d) override;
		// 轮子限制在车上,需要车子的中心点和车长/宽/高和轮子所属位置
		void AdjustPosition(DirectX::XMFLOAT3 targetCenter, float bodyWidth, float bodyLength, WheelPos wheelPos);
	};
	
	// 车身
	class Body : public VehiclePart
	{
	public:
		// 限制车身移动范围
		void AdjustPosition();
	};

	std::array<Wheel, 4> m_wheels;
	Body m_body;
	float m_bodyWidth;
	float m_bodyLength;
};



#endif
