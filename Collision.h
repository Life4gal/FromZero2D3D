//***************************************************************************************
// Collision.h by X_Jun(MKXJun) (C) 2018-2020 All Rights Reserved.
// Licensed under the MIT License.
//
// 提供一些封装好的对象和碰撞检测方法
// 注意：WireFrameData目前仍未经过稳定测试，未来有可能会移植到Geometry.h中
// Provide encapsulated collision classes and detection method.
//***************************************************************************************

#ifndef COLLISION_H
#define COLLISION_H

#include <DirectXCollision.h>
#include <vector>
#include "Vertex.h"
#include "Transform.h"

class Collision
{
public:

	// 线框顶点/索引数组
	struct WireFrameData
	{
		std::vector<VertexPosColor> vertexVec;		// 顶点数组
		std::vector<WORD> indexVec;					// 索引数组
	};

	//
	// 包围盒线框的创建
	//

	// 创建AABB盒线框
	static WireFrameData CreateBoundingBox(const DirectX::BoundingBox& box, const DirectX::XMFLOAT4& color);
	// 创建OBB盒线框
	static WireFrameData CreateBoundingOrientedBox(const DirectX::BoundingOrientedBox& box, const DirectX::XMFLOAT4& color);
	// 创建包围球线框
	static WireFrameData CreateBoundingSphere(const DirectX::BoundingSphere& sphere, const DirectX::XMFLOAT4& color, int slices = 20);
	// 创建视锥体线框
	static WireFrameData CreateBoundingFrustum(const DirectX::BoundingFrustum& frustum, const DirectX::XMFLOAT4& color);

	//
	// 三种等价的测试视锥体裁剪的方法，获取所有与视锥体碰撞的碰撞体对应的世界矩阵数组
	//

	// 视锥体裁剪
	static std::vector<DirectX::XMMATRIX> XM_CALLCONV FrustumCulling(
		const std::vector<DirectX::XMMATRIX>& matrices, const DirectX::BoundingBox& localBox, DirectX::FXMMATRIX view, DirectX::CXMMATRIX proj);
	// 视锥体裁剪2
	static std::vector<DirectX::XMMATRIX> XM_CALLCONV FrustumCulling2(
		const std::vector<DirectX::XMMATRIX>& matrices, const DirectX::BoundingBox& localBox, DirectX::FXMMATRIX view, DirectX::CXMMATRIX proj);
	// 视锥体裁剪3
	static std::vector<DirectX::XMMATRIX> XM_CALLCONV FrustumCulling3(
		const std::vector<DirectX::XMMATRIX>& matrices, const DirectX::BoundingBox& localBox, DirectX::FXMMATRIX view, DirectX::CXMMATRIX proj);

	// 视锥体裁剪
	static std::vector<Transform> XM_CALLCONV FrustumCulling(
		const std::vector<Transform>& transforms, const DirectX::BoundingBox& localBox, DirectX::FXMMATRIX view, DirectX::CXMMATRIX proj);
	// 视锥体裁剪2
	static std::vector<Transform> XM_CALLCONV FrustumCulling2(
		const std::vector<Transform>& transforms, const DirectX::BoundingBox& localBox, DirectX::FXMMATRIX view, DirectX::CXMMATRIX proj);
	// 视锥体裁剪3
	static std::vector<Transform> XM_CALLCONV FrustumCulling3(
		const std::vector<Transform>& transforms, const DirectX::BoundingBox& localBox, DirectX::FXMMATRIX view, DirectX::CXMMATRIX proj);


private:
	static WireFrameData CreateFromCorners(const DirectX::XMFLOAT3 (&corners)[8], const DirectX::XMFLOAT4& color);
};





#endif