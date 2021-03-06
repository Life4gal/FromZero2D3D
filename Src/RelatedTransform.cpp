#include "RelatedTransform.h"

using namespace DirectX;

RelatedTransform::RelatedTransform(const XMFLOAT3 scale, const XMFLOAT3 rotation, const XMFLOAT3 position)
	:
	BasicTransform(scale, rotation, position),
	m_absScale(scale),
	m_absRotationTranslation()
{
	// 初始的世界缩放与相对缩放相同
	// 初始的世界旋转平移矩阵与相对旋转平移矩阵相同
	XMStoreFloat4x4(&m_absRotationTranslation, GetRotationTranslationMatrix());
}

void RelatedTransform::ChangeChildParent(RelatedTransform* child, RelatedTransform* currParent, RelatedTransform* lastParent)
{
	// 乘上原父物体的世界缩放向量的倒数再乘上当前父物体的世界缩放向量
	// 我们之所以直接乘上世界缩放向量是因为父级物体可能依然有父级物体
	// 那么当前的自己物体还需要乘上更底层的父级缩放向量
	// 这时候就显现出了我们选择保存一个世界缩放向量的意义了
	// 
	// 缩放对顺序没有先后要求
	XMStoreFloat3(
		&child->m_absScale,
		// 当前的相对缩放向量
		child->GetScaleVector()
		// 乘上原父物体的世界缩放向量的倒数,如果没有原父物体乘上单位向量
		* (lastParent == nullptr ? XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f) : XMVectorReciprocal(lastParent->GetAbsScaleVector()))
		// 再乘上现父物体的世界缩放向量
		* currParent->GetAbsScaleVector()
	);

	// 乘上原父物体的世界旋转平移的逆矩阵再乘上当前父物体的世界旋转平移矩阵
	// 我们之所以直接乘上世界旋转平移矩阵是因为父级物体可能依然有父级物体
	// 那么当前的自己物体还需要乘上更底层的父级旋转平移矩阵
	// 这时候就显现出了我们选择保存一个世界旋转平移矩阵的意义了
	// 
	// 旋转平移不满足交换律,顺序必须正确
	XMStoreFloat4x4(
		&child->m_absRotationTranslation,
		// 当前的相对旋转平移矩阵
		child->GetRotationTranslationMatrix()
		// 乘上原父物体的世界旋转平移矩阵的逆矩阵,如果没有原父物体乘上单位矩阵
		* (lastParent == nullptr ? XMMatrixIdentity() : XMMatrixInverse(nullptr, lastParent->GetAbsRotationTranslationMatrix()))
		// 再乘上现父物体的世界旋转平移矩阵
		* currParent->GetAbsRotationTranslationMatrix()
	);
}

/*
void RelatedTransform::UpdateChildData(RelatedTransform* parent)
{
	// 我们应该先更新最底层的父物体信息然后再更新子物体以保证数据正确
	// 在每次绘制前我们都应该先更新这些信息
	
	// 最底层的父物体
	if(parent == nullptr)
	{
		// 更新保存的变换
		m_absScale = m_scale;
		XMStoreFloat4x4(&m_absRotationTranslation, GetRotationTranslationMatrix());
	}
	else
	{
		// 我们从子物体一路递归更新
		parent.UpdateChildData(parent->m_parent);
		// 然后再储存更新后的数据
		XMStoreFloat3(&m_absScale, GetScaleVector() * parent->GetAbsScaleVector());
	}
}
*/

//***********************************
// 相对于世界,绝对属性
// 继承自 BasicTransform 的接口都是相对属性
//

XMFLOAT3 RelatedTransform::GetAbsScaleFloat3() const
{
	return m_absScale;
}

XMVECTOR RelatedTransform::GetAbsScaleVector() const
{
	return XMLoadFloat3(&m_absScale);
}

XMFLOAT3 RelatedTransform::GetAbsRotationFloat3() const
{
	return GetEulerAnglesFromRotationTranslationFloat4X4(m_absRotationTranslation);
}

XMVECTOR RelatedTransform::GetAbsRotationVector() const
{
	XMFLOAT3 rotation = GetAbsRotationFloat3();
	return XMLoadFloat3(&rotation);
}

XMMATRIX RelatedTransform::GetAbsRotationMatrix() const
{
	return XMMatrixRotationRollPitchYawFromVector(GetAbsRotationVector());
}

XMFLOAT3 RelatedTransform::GetAbsPositionFloat3() const
{
	return GetTranslationFromRotationTranslationFloat4X4(m_absRotationTranslation);
}

XMVECTOR RelatedTransform::GetAbsPositionVector() const
{
	XMFLOAT3 position = GetAbsPositionFloat3();
	return XMLoadFloat3(&position);
}

XMMATRIX RelatedTransform::GetAbsPositionMatrix() const
{
	return XMMatrixTranslationFromVector(GetAbsPositionVector());
}

XMMATRIX RelatedTransform::GetAbsRotationTranslationMatrix() const
{
	return XMLoadFloat4x4(&m_absRotationTranslation);
}

XMVECTOR RelatedTransform::GetAbsRightAxisVector() const
{
	return GetAbsRotationTranslationMatrix().r[0];
}

XMVECTOR RelatedTransform::GetAbsUpAxisVector() const
{
	return GetAbsRotationTranslationMatrix().r[1];
}

XMVECTOR RelatedTransform::GetAbsForwardAxisVector() const
{
	return GetAbsRotationTranslationMatrix().r[2];
}

XMMATRIX RelatedTransform::GetAbsLocalToWorldMatrix() const
{
	return  XMMatrixScalingFromVector(XMLoadFloat3(&m_absScale)) * XMLoadFloat4x4(&m_absRotationTranslation);
}

XMMATRIX RelatedTransform::GetAbsWorldToLocalMatrix() const
{
	return XMMatrixInverse(nullptr, GetAbsLocalToWorldMatrix());
}
