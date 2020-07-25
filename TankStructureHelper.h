#ifndef TANKSTRUCTUREHELPER_H
#define TANKSTRUCTUREHELPER_H

#include "GameObject.h"
#include "Collision.h"

#include <array>

class BasicTankStructure
{
public:
	BasicTankStructure() = default;
	virtual ~BasicTankStructure() = default;

	BasicTankStructure(const BasicTankStructure& other) = default;
	BasicTankStructure(BasicTankStructure&& other) noexcept = default;
	BasicTankStructure& operator=(const BasicTankStructure& other) = default;
	BasicTankStructure& operator=(BasicTankStructure&& other) noexcept = default;

	// 初始化坦克
	virtual void Init(ID3D11Device* device) = 0;

	// 前后移动
	void Walk(const float d)
	{
		BasicTransform& transform = GetTankTransform();
		transform.Translate(transform.GetForwardAxisVector(), d);
		
		RotateWheels(d);
	}

	// 左右移动
	void Strafe(const float d)
	{
		// 转车身
		// @TODO 找一个比较合理的旋转速度
		GetTankTransform().Rotate(DirectX::XMVectorSet(0.0f, 0.1f * d, 0.0f, 0.0f));
	}
	
	// 开炮
	Ray Shoot() const
	{
		DirectX::XMMATRIX barrelLocalToWorldMatrix = GetBarrelLocalToWorldMatrix();

		DirectX::XMFLOAT3 position{};
		XMStoreFloat3(&position, barrelLocalToWorldMatrix.r[3]);
		return { position,  barrelLocalToWorldMatrix.r[1] };
	}
	
	// 转动炮管
	void Turn(const float d)
	{
		// 转底座
		// TODO 找一个比较合理的旋转速度
		GetBatteryTransform().Rotate(DirectX::XMVectorSet(0.0f, 0.25f * d, 0.0f, 0.0f));
	}

	// 获取位置
	DirectX::XMFLOAT3 GetPosition() const
	{
		return GetTankPosition();
	};

	// 设置位置
	void SetPosition(const DirectX::XMFLOAT3& position)
	{
		SetTankPosition(position);
	}

	// 调整位置
	void XM_CALLCONV AdjustPosition(DirectX::FXMVECTOR minCoordinate, DirectX::FXMVECTOR maxCoordinate)
	{
		DirectX::XMFLOAT3 position = GetTankPosition();
		
		DirectX::XMFLOAT3 adjustedPos{};
		XMStoreFloat3(&adjustedPos, DirectX::XMVectorClamp(XMLoadFloat3(&position), minCoordinate, maxCoordinate));
		
		SetTankPosition(adjustedPos);
	}

	// 绘制
	virtual void Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect) = 0;

private:
	// 获取坦克Transform
	virtual BasicTransform& GetTankTransform() = 0;

	virtual const BasicTransform& GetTankTransform() const = 0;

	virtual DirectX::XMFLOAT3 GetTankPosition() const = 0;
	
	virtual void SetTankPosition(const DirectX::XMFLOAT3& position) = 0;

	// 转动车轮
	virtual void RotateWheels(float d) = 0;

	// 获取炮台底座世界信息
	virtual DirectX::XMMATRIX GetBarrelLocalToWorldMatrix() const = 0;

	// 获取底座Transform
	virtual BasicTransform& GetBatteryTransform() = 0;

	virtual const BasicTransform& GetBatteryTransform() const = 0;
};

#endif
