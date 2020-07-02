#include "Transform.h"

using namespace DirectX;

Transform::Transform(const XMFLOAT3& scale, const XMFLOAT3& rotation, const XMFLOAT3& position)
	: m_Scale(scale), m_Rotation(rotation), m_Position(position)
{
}

XMFLOAT3 Transform::GetScale() const
{
	return m_Scale;
}

XMVECTOR Transform::GetScaleXM() const
{
	return XMLoadFloat3(&m_Scale);
}

XMFLOAT3 Transform::GetRotation() const
{
	return m_Rotation;
}

XMVECTOR Transform::GetRotationXM() const
{
	return XMLoadFloat3(&m_Rotation);
}

XMFLOAT3 Transform::GetPosition() const
{
	return m_Position;
}

XMVECTOR Transform::GetPositionXM() const
{
	return XMLoadFloat3(&m_Position);
}

XMVECTOR Transform::GetRightAxisXM() const
{
	return XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation)).r[0];
}

XMVECTOR Transform::GetUpAxisXM() const
{
	return XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation)).r[1];
}

XMVECTOR Transform::GetForwardAxisXM() const
{
	return XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation)).r[2];
}

XMMATRIX Transform::GetLocalToWorldMatrixXM() const
{
	const XMVECTOR scaleVec = XMLoadFloat3(&m_Scale);
	const XMVECTOR rotationVec = XMLoadFloat3(&m_Rotation);
	const XMVECTOR positionVec = XMLoadFloat3(&m_Position);
	const XMMATRIX world = XMMatrixScalingFromVector(scaleVec) * XMMatrixRotationRollPitchYawFromVector(rotationVec) * XMMatrixTranslationFromVector(positionVec);
	return world;
}

XMMATRIX Transform::GetWorldToLocalMatrixXM() const
{
	return XMMatrixInverse(nullptr, GetLocalToWorldMatrixXM());
}

void Transform::SetScale(const XMFLOAT3& scale)
{
	m_Scale = scale;
}

void Transform::SetScale(const XMVECTOR& scale)
{
	 XMStoreFloat3(&m_Scale, scale);
}

void Transform::SetScale(float x, float y, float z)
{
	m_Scale = XMFLOAT3(x, y, z);
}

void Transform::SetRotation(const XMFLOAT3& eulerAnglesInRadian)
{
	m_Rotation = eulerAnglesInRadian;
}

void Transform::SetRotation(const XMVECTOR& eulerAnglesInRadian)
{
	XMStoreFloat3(&m_Rotation, eulerAnglesInRadian);
}

void Transform::SetRotation(float x, float y, float z)
{
	m_Rotation = XMFLOAT3(x, y, z);
}

void Transform::SetPosition(const XMFLOAT3& position)
{
	m_Position = position;
}

void Transform::SetPosition(const XMVECTOR& position)
{
	XMStoreFloat3(&m_Position, position);
}

void Transform::SetPosition(float x, float y, float z)
{
	m_Position = XMFLOAT3(x, y, z);
}

void Transform::Rotate(const XMVECTOR& eulerAnglesInRadian)
{
	// 基于旋转欧拉角的旋转，只需要更新欧拉角即可
	const XMVECTOR newRotationVec = XMVectorAdd(XMLoadFloat3(&m_Rotation), eulerAnglesInRadian);
	XMStoreFloat3(&m_Rotation, newRotationVec);
}

void Transform::RotateAxis(const XMVECTOR& axis, float radian)
{
	// 绕轴旋转，先根据当前欧拉角得到旋转矩阵，然后更新，最后还原欧拉角
	XMMATRIX R = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation));
	
	R *= XMMatrixRotationAxis(axis, radian);
	
	XMFLOAT4X4 rotMatrix{};
	XMStoreFloat4x4(&rotMatrix, R);
	
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatrix);
}

void Transform::RotateAround(const XMVECTOR& point, const XMVECTOR& axis, float radian) const
{
	// 基于某一点为旋转中心进行绕轴旋转的实现过程稍微有点复杂。
	// 首先根据已有变换算出旋转矩阵*平移矩阵，然后将旋转中心平移到原点（这两步平移可以合并），再进行旋转，最后再平移回旋转中心
	const XMVECTOR rotation = XMLoadFloat3(&m_Rotation);
	const XMVECTOR position = XMLoadFloat3(&m_Position);

	// 以point作为原点进行旋转
	XMMATRIX RT = XMMatrixRotationRollPitchYawFromVector(rotation) * XMMatrixTranslationFromVector(position - point);
	
	RT *= XMMatrixRotationAxis(axis, radian);
	RT *= XMMatrixTranslationFromVector(point);
	
	XMFLOAT4X4 rotMatrix{};
	XMStoreFloat4x4(&rotMatrix, RT);
}

void Transform::Translate(const XMVECTOR& direction, float magnitude)
{
	const XMVECTOR directionVec = XMVector3Normalize(direction);
	const XMVECTOR newPosition = XMVectorMultiplyAdd(XMVectorReplicate(magnitude), directionVec, XMLoadFloat3(&m_Position));
	XMStoreFloat3(&m_Position, newPosition);
}

void Transform::LookAt(const XMFLOAT3& target, const XMFLOAT3& up)
{
	/*
		若已知Q为摄像机的位置，T为摄像机对准的观察目标点，j为世界空间“向上”方向的单位向量。
		以平面xOz作为场景中的“地平面”，并以世界空间的y轴作为摄像机“向上”的方向。因此，j = (0,1,0)仅是平行于世界空间中y轴的一个单位向量，虚拟摄像机的观察方向为：

		||vector||表示取向量的模长,做分母用于取单位向量
		虚拟摄像机局部空间的z轴:
		w = T−Q / ||T−Q||
		虚拟摄像机局部空间的x轴：
		u = j×w / ||j×w||		// 这里用的是叉积(外积)
		虚拟摄像机局部空间的y轴：
		v = w×u
		因为w和u是互相正交的单位向量，所以v也必为单位向量。因此我们也无须对向量v进行规范化处理了。

		针对上述计算观察矩阵的处理流程提供了以下函数：
		// 观察矩阵
		XMMATRIX XMMatrixLookAtLH(  // 输出视图变换矩阵V
			FXMVECTOR EyePosition,      // 输入摄影机坐标
			FXMVECTOR FocusPosition,    // 输入摄影机焦点坐标
			FXMVECTOR UpDirection);     // 输入摄影机上朝向坐标

		// 透视投影矩阵
		XMMATRIX XMMatrixPerspectiveFovLH( // 返回投影矩阵
			FLOAT FovAngleY,                   // 中心垂直弧度
			FLOAT AspectRatio,                 // 宽高比
			FLOAT NearZ,                       // 近平面距离
			FLOAT FarZ);                       // 远平面距离
	 */
	const XMMATRIX View = XMMatrixLookAtLH(
		XMLoadFloat3(&m_Position), 
		XMLoadFloat3(&target), 
		XMLoadFloat3(&up)
	);

	XMFLOAT4X4 rotMatrix{};
	XMStoreFloat4x4(&rotMatrix, XMMatrixInverse(nullptr, View));
	
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatrix);
}

void Transform::LookTo(const XMFLOAT3& direction, const XMFLOAT3& up)
{
	const XMMATRIX View = XMMatrixLookToLH(
		XMLoadFloat3(&m_Position), 
		XMLoadFloat3(&direction), 
		XMLoadFloat3(&up)
	);
	
	XMFLOAT4X4 rotMatrix{};
	XMStoreFloat4x4(&rotMatrix, XMMatrixInverse(nullptr, View));
	
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatrix);
}

XMFLOAT3 Transform::GetEulerAnglesFromRotationMatrix(const XMFLOAT4X4& rotationMatrix)
{
	// 通过旋转矩阵反求欧拉角
	float c = sqrtf(1.0f - rotationMatrix(2, 1) * rotationMatrix(2, 1));
	// 防止r[2][1]出现大于1的情况
	if (isnan(c))
	{
		c = 0.0f;
	}
		
	const XMFLOAT3 rotation
	{
		/*
			规定M00为矩阵第一行第一列元素
			
			θx = atan2(M21, sqrt(1 - M21^2))
			θy = atan2(-M20, M22)
			θz = atan2(-M01, M11)

			在还原欧拉角的时候，由于浮点数的精度问题，
			可能会导致M21莫名其妙地大于1，从而导致根式部分无定义,
			此时令 sqrt(1 - M21^2) = 0
		 */
		atan2f(-rotationMatrix(2, 1), c),
		atan2f(rotationMatrix(2, 0), rotationMatrix(2, 2)),
		atan2f(rotationMatrix(0, 1), rotationMatrix(1, 1))
	};
	
	return rotation;
}
