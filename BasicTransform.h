//***************************************************************************************
// Author: X_Jun(MKXJun)(MIT License)
//
// Modified By: life4gal(NiceT)(MIT License)
//
// 基于 X_Jun 的 Transform ,做了较大改动
// 
// 描述对象缩放、旋转(欧拉角为基础)、平移
// Provide 1st person(free view) and 3rd person cameras.
//***************************************************************************************

/*
 * 对于 XMVECTOR 类型的 常引用 参数
 * 前三个用 FXMVECTOR
 * 第四个用 GXMVECTOR
 * 第五和六个用 HXMVECTOR
 * 其余用 CXMVECTOR
 *
 * 注意: 只算 XMVECTOR 类型的 常引用 参数,其它类型参数不计数
 *	
 * 回调使用 XM_CALLCONV
 * 
 */

#ifndef BASICTRANSFORM_H
#define BASICTRANSFORM_H

#include <DirectXMath.h>

class BasicTransform
{
public:
	explicit BasicTransform(DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 position);

	virtual ~BasicTransform() = default;

	BasicTransform(const BasicTransform & other) = default;
	BasicTransform(BasicTransform && other) noexcept = default;
	BasicTransform& operator=(const BasicTransform & other) = default;
	BasicTransform& operator=(BasicTransform && other) noexcept = default;

	// 获取对象缩放比例
	DirectX::XMFLOAT3 GetScaleFloat3() const;
	DirectX::XMVECTOR GetScaleVector() const;
	
	// 获取对象欧拉角(弧度制)
	// 对象以Z-X-Y轴顺序旋转
	DirectX::XMFLOAT3 GetRotationFloat3() const;
	DirectX::XMVECTOR GetRotationVector() const;
	DirectX::XMMATRIX GetRotationMatrix() const;
	
	// 获取对象位置
	DirectX::XMFLOAT3 GetPositionFloat3() const;
	DirectX::XMVECTOR GetPositionVector() const;
	DirectX::XMMATRIX GetPositionMatrix() const;

	// 获取旋转平移矩阵
	DirectX::XMMATRIX GetRotationTranslationMatrix() const;
	
	// 获取右方向轴
	DirectX::XMVECTOR GetRightAxisVector() const;
	// 获取上方向轴
	DirectX::XMVECTOR GetUpAxisVector() const;
	// 获取前方向轴
	DirectX::XMVECTOR GetForwardAxisVector() const;

	// 获取世界变换矩阵
	DirectX::XMMATRIX GetLocalToWorldMatrix() const;
	// 获取逆世界变换矩阵
	DirectX::XMMATRIX GetWorldToLocalMatrix() const;

	// 设置对象缩放比例
	void SetScale(const DirectX::XMFLOAT3& scale);
	void XM_CALLCONV SetScale(DirectX::FXMVECTOR scale);
	void SetScale(float x, float y, float z);

	// 设置对象欧拉角(弧度制)
	// 对象将以Z-X-Y轴顺序旋转
	void SetRotation(const DirectX::XMFLOAT3& eulerAnglesInRadian);
	void XM_CALLCONV SetRotation(DirectX::FXMVECTOR eulerAnglesInRadian);
	void SetRotation(float x, float y, float z);

	// 设置对象位置
	void SetPosition(const DirectX::XMFLOAT3& position);
	void XM_CALLCONV SetPosition(DirectX::FXMVECTOR position);
	void SetPosition(float x, float y, float z);
	
	// 指定欧拉角旋转对象
	void XM_CALLCONV Rotate(DirectX::FXMVECTOR eulerAnglesInRadian);
	// 指定以原点为中心绕轴旋转
	void XM_CALLCONV RotateAxis(DirectX::FXMVECTOR axis, float radian);
	// 指定以point为旋转中心绕轴旋转
	void XM_CALLCONV RotateAround(DirectX::FXMVECTOR point, DirectX::FXMVECTOR axis, float radian);

	// 沿着某一方向平移
	void XM_CALLCONV Translate(DirectX::FXMVECTOR direction, float magnitude);

	// 观察某一点
	void LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up = { 0.0f, 1.0f, 0.0f });
	// 沿着某一方向观察
	void LookTo(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& up = { 0.0f, 1.0f, 0.0f });

protected:
	// 从旋转平移矩阵获取旋转欧拉角
	static DirectX::XMFLOAT3 GetEulerAnglesFromRotationTranslationFloat4X4(const DirectX::XMFLOAT4X4& rotationTranslationFloat4X4);
	// 从旋转平移矩阵获取平移向量
	static DirectX::XMFLOAT3 GetTranslationFromRotationTranslationFloat4X4(const DirectX::XMFLOAT4X4& rotationTranslationFloat4X4);

	DirectX::XMFLOAT3 m_scale;				// 缩放
	DirectX::XMFLOAT3 m_rotation;			// 旋转欧拉角(弧度制)
	DirectX::XMFLOAT3 m_position;			// 位置
};

#endif


