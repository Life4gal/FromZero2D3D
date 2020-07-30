#include "BasicTransform.h"

using namespace DirectX;

BasicTransform::BasicTransform()
	:
	m_scale{ 1.0f, 1.0f, 1.0f },
	m_rotation{},
	m_position{}
{
}

BasicTransform::BasicTransform(const XMFLOAT3& scale, const XMFLOAT3& rotation, const XMFLOAT3& position)
	:
	m_scale(scale),
	m_rotation(rotation),
	m_position(position)
{
}

XMFLOAT3 BasicTransform::GetScaleFloat3() const
{
	return m_scale;
}

XMVECTOR BasicTransform::GetScaleVector() const
{
	return XMLoadFloat3(&m_scale);
}

XMMATRIX BasicTransform::GetScaleMatrix() const
{
	return XMMatrixScalingFromVector(XMLoadFloat3(&m_scale));
}

XMFLOAT3 BasicTransform::GetRotationFloat3() const
{
	return m_rotation;
}

XMVECTOR BasicTransform::GetRotationVector() const
{
	return XMLoadFloat3(&m_rotation);
}

XMMATRIX BasicTransform::GetRotationMatrix() const
{
	return XMMatrixRotationRollPitchYawFromVector(GetRotationVector());
}

XMFLOAT3 BasicTransform::GetPositionFloat3() const
{
	return m_position;
}

XMVECTOR BasicTransform::GetPositionVector() const
{
	return XMLoadFloat3(&m_position);
}

XMMATRIX BasicTransform::GetPositionMatrix() const
{
	return XMMatrixTranslationFromVector(XMLoadFloat3(&m_position));
}

XMMATRIX BasicTransform::GetRotationTranslationMatrix() const
{
	return GetRotationMatrix() * GetPositionMatrix();
}

XMVECTOR BasicTransform::GetRightAxisVector() const
{
	return GetRotationMatrix().r[0];
}

XMVECTOR BasicTransform::GetUpAxisVector() const
{
	return GetRotationMatrix().r[1];
}

XMVECTOR BasicTransform::GetForwardAxisVector() const
{
	return GetRotationMatrix().r[2];
}

XMMATRIX BasicTransform::GetLocalToWorldMatrix() const
{
	return
		XMMatrixScalingFromVector(XMLoadFloat3(&m_scale)) *
		XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rotation)) *
		XMMatrixTranslationFromVector(XMLoadFloat3(&m_position));
}

XMMATRIX BasicTransform::GetWorldToLocalMatrix() const
{
	return XMMatrixInverse(nullptr, GetLocalToWorldMatrix());
}

void BasicTransform::SetScale(const XMFLOAT3& scale)
{
	m_scale = scale;
}

void XM_CALLCONV BasicTransform::SetScale(FXMVECTOR scale)
{
	XMStoreFloat3(&m_scale, scale);
}

void BasicTransform::SetScale(const float x, const float y, const float z)
{
	m_scale = XMFLOAT3(x, y, z);
}

void BasicTransform::SetRotation(const XMFLOAT3& eulerAnglesInRadian)
{
	m_rotation = eulerAnglesInRadian;
}

void XM_CALLCONV BasicTransform::SetRotation(FXMVECTOR eulerAnglesInRadian)
{
	XMStoreFloat3(&m_rotation, eulerAnglesInRadian);
}

void BasicTransform::SetRotation(const float x, const float y, const float z)
{
	m_rotation = XMFLOAT3(x, y, z);
}

void BasicTransform::SetPosition(const XMFLOAT3& position)
{
	m_position = position;
}

void XM_CALLCONV BasicTransform::SetPosition(FXMVECTOR position)
{
	XMStoreFloat3(&m_position, position);
}

void BasicTransform::SetPosition(const float x, const float y, const float z)
{
	m_position = XMFLOAT3(x, y, z);
}

void XM_CALLCONV BasicTransform::Rotate(FXMVECTOR eulerAnglesInRadian)
{
	// 基于旋转欧拉角的旋转，只需要更新欧拉角即可
	XMStoreFloat3(&m_rotation, XMVectorAdd(XMLoadFloat3(&m_rotation), eulerAnglesInRadian));
}

void XM_CALLCONV BasicTransform::RotateAxis(FXMVECTOR axis, const float radian)
{
	// 绕轴旋转，先根据当前欧拉角得到旋转矩阵，然后更新，最后还原欧拉角
	XMFLOAT4X4 rotationTranslationFloat4X4{};
	XMStoreFloat4x4(&rotationTranslationFloat4X4, GetRotationMatrix() * XMMatrixRotationAxis(axis, radian));

	m_rotation = GetEulerAnglesFromRotationTranslationFloat4X4(rotationTranslationFloat4X4);
}

void XM_CALLCONV BasicTransform::RotateAround(FXMVECTOR point, FXMVECTOR axis, const float radian)
{
	// 基于某一点为旋转中心进行绕轴旋转
	// 首先根据已有变换算出旋转矩阵*平移矩阵，然后将旋转中心平移到原点，再进行旋转，最后再平移回旋转中心

	// 将旋转中心平移到原点
	XMMATRIX rotationTranslationMatrix = GetRotationTranslationMatrix() * XMMatrixTranslationFromVector(-point);
	// 进行旋转
	rotationTranslationMatrix *= XMMatrixRotationAxis(axis, radian);
	// 再平移回旋转中心
	rotationTranslationMatrix *= XMMatrixTranslationFromVector(point);

	XMFLOAT4X4 rotationTranslationFloat4X4{};
	XMStoreFloat4x4(&rotationTranslationFloat4X4, rotationTranslationMatrix);

	m_rotation = GetEulerAnglesFromRotationTranslationFloat4X4(rotationTranslationFloat4X4);
	XMStoreFloat3(&m_position, rotationTranslationMatrix.r[3]);
}

void XM_CALLCONV BasicTransform::Translate(FXMVECTOR direction, const float magnitude)
{
	XMStoreFloat3(&m_position, XMVectorMultiplyAdd(XMVectorReplicate(magnitude), XMVector3Normalize(direction), GetPositionVector()));
}

void XM_CALLCONV BasicTransform::LookAt(FXMVECTOR target, FXMVECTOR up)
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
	
	XMFLOAT4X4 rotationTranslationFloat4X4{};
	// 获取逆观察矩阵
	XMStoreFloat4x4(&rotationTranslationFloat4X4,
		XMMatrixInverse(
			nullptr,
			XMMatrixLookAtLH(
				GetPositionVector(),
				target,
				up
			)
		)
	);

	m_rotation = GetEulerAnglesFromRotationTranslationFloat4X4(rotationTranslationFloat4X4);
}

void XM_CALLCONV BasicTransform::LookTo(FXMVECTOR direction, FXMVECTOR up)
{
	XMFLOAT4X4 rotationTranslationFloat4X4{};
	// 获取逆观察矩阵
	XMStoreFloat4x4(&rotationTranslationFloat4X4,
		XMMatrixInverse(
			nullptr,
			XMMatrixLookToLH(
				GetPositionVector(),
				direction,
				up
			)
		)
	);

	m_rotation = GetEulerAnglesFromRotationTranslationFloat4X4(rotationTranslationFloat4X4);
}

XMFLOAT3 BasicTransform::GetEulerAnglesFromRotationTranslationFloat4X4(const XMFLOAT4X4& rotationTranslationFloat4X4)
{
	// 通过旋转矩阵反求欧拉角

	/*
		在还原欧拉角的时候，由于浮点数的精度问题，
		可能会导致M21莫名其妙地大于1，从而导致根式部分无定义,
		此时令 sqrt(1 - M21^2) = 0
	 */
	float c = sqrtf(1.0f - rotationTranslationFloat4X4(2, 1) * rotationTranslationFloat4X4(2, 1));
	// 防止r[2][1]出现大于1的情况
	if (isnan(c))
	{
		c = 0.0f;
	}

	return
	{
		/*
			规定M00为矩阵第一行第一列元素

			θx = atan2(M21, sqrt(1 - M21^2))
			θy = atan2(-M20, M22)
			θz = atan2(-M01, M11)


		 */
		atan2f(-rotationTranslationFloat4X4(2, 1), c),
		atan2f(rotationTranslationFloat4X4(2, 0), rotationTranslationFloat4X4(2, 2)),
		atan2f(rotationTranslationFloat4X4(0, 1), rotationTranslationFloat4X4(1, 1))
	};
}

XMFLOAT3 BasicTransform::GetTranslationFromRotationTranslationFloat4X4(const XMFLOAT4X4& rotationTranslationFloat4X4)
{
	return
	{
		rotationTranslationFloat4X4(3, 0),
		rotationTranslationFloat4X4(3, 1),
		rotationTranslationFloat4X4(3, 2),
	};
}
