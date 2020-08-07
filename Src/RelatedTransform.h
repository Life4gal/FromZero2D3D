#ifndef RELATEDTRANSFORM_H
#define RELATEDTRANSFORM_H

#include "BasicTransform.h"

/*
 * 我们保存子物体到世界的变换属性并不能为我们减少每次绘制的代价,甚至正好相反
 * 这个类存在的意义应该只是方便那些需要频繁访问相对于世界的一些信息的类
 * 对于不需要这些数据的类来说这是一个很大的负担,毕竟我们的大小是原来三倍
 */
class RelatedTransform final : public BasicTransform
{
public:
	explicit RelatedTransform(DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f }, DirectX::XMFLOAT3 rotation = {}, DirectX::XMFLOAT3 position = {});

private:
	// 这些矩阵是从当前对象到最底层父对象累乘的结果,这样该对象的子对象只需要再乘上当前矩阵即可求出相对世界的矩阵
	DirectX::XMFLOAT3 m_absScale;
	DirectX::XMFLOAT4X4 m_absRotationTranslation;

	//***********************************
	// 更新世界矩阵辅助函数
	//

	// 在更换父物体时对子物体世界矩阵的更新
	// 对于一个父物体而言,初始的子物体容器为空,所以只要我们每次添加子物体时适时地更新子物体世界矩阵,就不需要额外的更新操作了
	static void ChangeChildParent(RelatedTransform* child, RelatedTransform* currParent, RelatedTransform* lastParent = nullptr);

public:
	// 更新属性
	// TODO 我们没有对 RelatedTransform 实现父子关联链,实现在使用RelatedTransform上层对象,所以这个函数无法使用
	// void UpdateChildData(RelatedTransform* parent);
	
	//***********************************
	// 相对于世界,绝对属性
	// 继承自 BasicTransform 的接口都是相对属性
	// 

	// 获取对象缩放比例
	DirectX::XMFLOAT3 GetAbsScaleFloat3() const;
	DirectX::XMVECTOR GetAbsScaleVector() const;

	// 获取对象欧拉角(弧度制)
	// 对象以Z-X-Y轴顺序旋转
	DirectX::XMFLOAT3 GetAbsRotationFloat3() const;
	DirectX::XMVECTOR GetAbsRotationVector() const;
	DirectX::XMMATRIX GetAbsRotationMatrix() const;

	// 获取对象位置
	DirectX::XMFLOAT3 GetAbsPositionFloat3() const;
	DirectX::XMVECTOR GetAbsPositionVector() const;
	DirectX::XMMATRIX GetAbsPositionMatrix() const;

	// 获取绝对旋转平移矩阵
	DirectX::XMMATRIX GetAbsRotationTranslationMatrix() const;

	// 获取绝对右方向轴
	DirectX::XMVECTOR GetAbsRightAxisVector() const;
	// 获取绝对上方向轴
	DirectX::XMVECTOR GetAbsUpAxisVector() const;
	// 获取绝对前方向轴
	DirectX::XMVECTOR GetAbsForwardAxisVector() const;

	// 获取绝对世界变换矩阵
	DirectX::XMMATRIX GetAbsLocalToWorldMatrix() const;
	// 获取绝对逆世界变换矩阵
	DirectX::XMMATRIX GetAbsWorldToLocalMatrix() const;
};

#endif
