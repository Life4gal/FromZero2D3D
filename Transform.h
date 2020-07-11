//***************************************************************************************
// Author: X_Jun(MKXJun)(MIT License)
//
// Modified By: life4gal(NiceT)(MIT License)
//
// 描述对象缩放、旋转(欧拉角为基础)、平移
// Provide 1st person(free view) and 3rd person cameras.
//***************************************************************************************

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <DirectXMath.h>

class Transform
{
public:
	Transform();
	Transform(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& position);

	// 获取对象缩放比例
	DirectX::XMFLOAT3 GetScale() const;
	DirectX::XMVECTOR GetScaleXM() const;
	// 获取对象欧拉角(弧度制)
	// 对象以Z-X-Y轴顺序旋转
	DirectX::XMFLOAT3 GetRotation() const;
	DirectX::XMVECTOR GetRotationXM() const;
	// 获取对象位置
	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMVECTOR GetPositionXM() const;

	// 获取右方向轴
	DirectX::XMVECTOR GetRightAxisXM() const;
	// 获取上方向轴
	DirectX::XMVECTOR GetUpAxisXM() const;
	// 获取前方向轴
	DirectX::XMVECTOR GetForwardAxisXM() const;

	// 获取世界变换矩阵
	DirectX::XMMATRIX GetLocalToWorldMatrixXM() const;
	// 获取逆世界变换矩阵
	DirectX::XMMATRIX GetWorldToLocalMatrixXM() const;

	// 设置对象缩放比例
	void SetScale(const DirectX::XMFLOAT3& scale);
	void SetScale(const DirectX::XMVECTOR& scale);
	void SetScale(float x, float y, float z);

	// 设置对象欧拉角(弧度制)
	// 对象将以Z-X-Y轴顺序旋转
	void SetRotation(const DirectX::XMFLOAT3& eulerAnglesInRadian);
	void SetRotation(const DirectX::XMVECTOR& eulerAnglesInRadian);
	void SetRotation(float x, float y, float z);

	// 设置对象位置
	void SetPosition(const DirectX::XMFLOAT3& position);
	void SetPosition(const DirectX::XMVECTOR& position);
	void SetPosition(float x, float y, float z);
	
	// 指定欧拉角旋转对象
	void Rotate(const DirectX::XMVECTOR& eulerAnglesInRadian);
	// 指定以原点为中心绕轴旋转
	void RotateAxis(const DirectX::XMVECTOR& axis, float radian);
	// 指定以point为旋转中心绕轴旋转
	void RotateAround(const DirectX::XMVECTOR& point, const DirectX::XMVECTOR& axis, float radian) const;

	// 沿着某一方向平移
	void Translate(const DirectX::XMVECTOR& direction, float magnitude);

	// 观察某一点
	void LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up = { 0.0f, 1.0f, 0.0f });
	// 沿着某一方向观察
	void LookTo(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& up = { 0.0f, 1.0f, 0.0f });

private:
	// 从旋转矩阵获取旋转欧拉角
	static DirectX::XMFLOAT3 GetEulerAnglesFromRotationMatrix(const DirectX::XMFLOAT4X4& rotationMatrix);

	DirectX::XMFLOAT3 m_scale;				// 缩放
	DirectX::XMFLOAT3 m_rotation;			// 旋转欧拉角(弧度制)
	DirectX::XMFLOAT3 m_position;			// 位置
};

#endif


