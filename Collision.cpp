#include "Collision.h"

using namespace DirectX;

Collision::WireFrameData Collision::CreateBoundingBox(const BoundingBox& box, const XMFLOAT4& color)
{
	XMFLOAT3 corners[8];
	box.GetCorners(corners);
	return CreateFromCorners(corners, color);
}

Collision::WireFrameData Collision::CreateBoundingOrientedBox(const BoundingOrientedBox& box, const XMFLOAT4& color)
{
	XMFLOAT3 corners[8];
	box.GetCorners(corners);
	return CreateFromCorners(corners, color);
}

Collision::WireFrameData Collision::CreateBoundingSphere(const BoundingSphere& sphere, const XMFLOAT4& color, const int slices)
{
	WireFrameData data;
	const XMVECTOR center = XMLoadFloat3(&sphere.Center);
	
	float theta = 0.0f;
	for (int i = 0; i < slices; ++i)
	{
		XMVECTOR posVec = XMVector3Transform(center + XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), XMMatrixRotationY(theta));
		XMFLOAT3 pos{};
		
		XMStoreFloat3(&pos, posVec);
		data.vertexVec.emplace_back(pos, color);
		posVec = XMVector3Transform(center + XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), XMMatrixRotationZ(theta));
		XMStoreFloat3(&pos, posVec);
		data.vertexVec.emplace_back(pos, color);
		posVec = XMVector3Transform(center + XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), XMMatrixRotationX(theta));
		XMStoreFloat3(&pos, posVec);
		data.vertexVec.emplace_back(pos, color);
		theta += XM_2PI / static_cast<float>(slices);
	}
	for (int i = 0; i < slices; ++i)
	{
		data.indexVec.push_back(i * 3);
		data.indexVec.push_back((i + 1) % slices * 3);

		data.indexVec.push_back(i * 3 + 1);
		data.indexVec.push_back((i + 1) % slices * 3 + 1);

		data.indexVec.push_back(i * 3 + 2);
		data.indexVec.push_back((i + 1) % slices * 3 + 2);
	}

	return data;
}

Collision::WireFrameData Collision::CreateBoundingFrustum(const BoundingFrustum& frustum, const XMFLOAT4& color)
{
	XMFLOAT3 corners[8];
	frustum.GetCorners(corners);
	return CreateFromCorners(corners, color);
}

std::vector<XMMATRIX> XM_CALLCONV Collision::FrustumCulling(
	const std::vector<XMMATRIX>& matrices,const BoundingBox& localBox, FXMMATRIX view, CXMMATRIX proj)
{
	std::vector<XMMATRIX> acceptedData;

	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, proj);
	const XMMATRIX invView = XMMatrixInverse(nullptr, view);
	// 将视锥体从局部坐标系变换到世界坐标系中
	frustum.Transform(frustum, invView);

	BoundingOrientedBox localOrientedBox, orientedBox;
	BoundingOrientedBox::CreateFromBoundingBox(localOrientedBox, localBox);
	for (auto& mat : matrices)
	{
		// 将有向包围盒从局部坐标系变换到世界坐标系中
		localOrientedBox.Transform(orientedBox, mat);
		// 相交检测
		if (frustum.Intersects(orientedBox))
			acceptedData.push_back(mat);
	}

	return acceptedData;
}

std::vector<XMMATRIX> XM_CALLCONV Collision::FrustumCulling2(
	const std::vector<XMMATRIX>& matrices,const BoundingBox& localBox, FXMMATRIX view, CXMMATRIX proj)
{
	std::vector<XMMATRIX> acceptedData;

	BoundingFrustum frustum, localFrustum;
	BoundingFrustum::CreateFromMatrix(frustum, proj);
	const XMMATRIX invView = XMMatrixInverse(nullptr, view);
	for (auto& mat : matrices)
	{
		const XMMATRIX invWorld = XMMatrixInverse(nullptr, mat);

		// 将视锥体从观察坐标系(或局部坐标系)变换到物体所在的局部坐标系中
		frustum.Transform(localFrustum, invView * invWorld);
		// 相交检测
		if (localFrustum.Intersects(localBox))
			acceptedData.push_back(mat);
	}

	return acceptedData;
}

std::vector<XMMATRIX> XM_CALLCONV Collision::FrustumCulling3(
	const std::vector<XMMATRIX>& matrices,const BoundingBox& localBox, FXMMATRIX view, CXMMATRIX proj)
{
	std::vector<XMMATRIX> acceptedData;

	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, proj);

	BoundingOrientedBox localOrientedBox, orientedBox;
	BoundingOrientedBox::CreateFromBoundingBox(localOrientedBox, localBox);
	for (auto& mat : matrices)
	{
		// 将有向包围盒从局部坐标系变换到视锥体所在的局部坐标系(观察坐标系)中
		localOrientedBox.Transform(orientedBox, mat * view);
		// 相交检测
		if (frustum.Intersects(orientedBox))
			acceptedData.push_back(mat);
	}

	return acceptedData;
}

/*
	由投影矩阵构造出来的视锥体包围盒也位于自身局部坐标系中，
	而观察矩阵实质上是从世界矩阵变换到视锥体所处的局部坐标系中。
	因此，我们可以使用观察矩阵的逆矩阵，将视锥体包围盒也变换到世界空间中。
	这样就好似物体与视锥体都位于世界空间中，可以进行碰撞检测了
 */
std::vector<Transform> XM_CALLCONV Collision::FrustumCulling(
	const std::vector<Transform>& transforms, const BoundingBox& localBox, FXMMATRIX view, CXMMATRIX proj)
{
	std::vector<Transform> acceptedData;

	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, proj);

	BoundingOrientedBox localOrientedBox, orientedBox;
	BoundingOrientedBox::CreateFromBoundingBox(localOrientedBox, localBox);
	for (auto& transform : transforms)
	{
		XMMATRIX world = transform.GetLocalToWorldMatrixXM();
		// 将有向包围盒从局部坐标系变换到视锥体所在的局部坐标系(观察坐标系)中
		localOrientedBox.Transform(orientedBox, world * view);
		// 相交检测
		if (frustum.Intersects(orientedBox))
			acceptedData.push_back(transform);
	}

	return acceptedData;
}

/*
	分别对观察矩阵和世界变换矩阵求逆，
	然后使用观察逆矩阵将视锥体从自身坐标系搬移到世界坐标系，
	再使用世界变换的逆矩阵将其从世界坐标系搬移到物体自身坐标系来与物体进行碰撞检测
 */
std::vector<Transform> XM_CALLCONV Collision::FrustumCulling2(
	const std::vector<Transform>& transforms, const DirectX::BoundingBox& localBox, FXMMATRIX view, CXMMATRIX proj)
{
	std::vector<Transform> acceptedData;

	BoundingFrustum frustum, localFrustum;
	BoundingFrustum::CreateFromMatrix(frustum, proj);
	const XMMATRIX invView = XMMatrixInverse(nullptr, view);
	for (auto& t : transforms)
	{
		const XMMATRIX world = t.GetLocalToWorldMatrixXM();
		const XMMATRIX invWorld = XMMatrixInverse(nullptr, world);

		// 将视锥体从观察坐标系(或局部坐标系)变换到物体所在的局部坐标系中
		frustum.Transform(localFrustum, invView * invWorld);
		// 相交检测
		if (localFrustum.Intersects(localBox))
			acceptedData.push_back(t);
	}

	return acceptedData;
}

/*
	先将物体从局部坐标系搬移到世界坐标系，
	然后再用观察矩阵将其搬移到视锥体自身的局部坐标系来与视锥体进行碰撞检测
 */
std::vector<Transform> XM_CALLCONV Collision::FrustumCulling3(
	const std::vector<Transform>& transforms, const DirectX::BoundingBox& localBox, FXMMATRIX view, CXMMATRIX proj)
{
	std::vector<Transform> acceptedData;

	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, proj);

	BoundingOrientedBox localOrientedBox, orientedBox;
	BoundingOrientedBox::CreateFromBoundingBox(localOrientedBox, localBox);
	for (auto& transform : transforms)
	{
		XMMATRIX world = transform.GetLocalToWorldMatrixXM();
		// 将有向包围盒从局部坐标系变换到视锥体所在的局部坐标系(观察坐标系)中
		localOrientedBox.Transform(orientedBox, world * view);
		// 相交检测
		if (frustum.Intersects(orientedBox))
			acceptedData.push_back(transform);
	}

	return acceptedData;
}

Collision::WireFrameData Collision::CreateFromCorners(const DirectX::XMFLOAT3(&corners)[8], const DirectX::XMFLOAT4& color)
{
	WireFrameData data;
	// AABB/OBB顶点索引如下    视锥体顶点索引如下
	//     3_______2             4__________5
	//    /|      /|             |\        /|
	//  7/_|____6/ |             | \      / |
	//  |  |____|__|            7|_0\____/1_|6
	//  | /0    | /1              \ |    | /
	//  |/______|/                 \|____|/
	//  4       5                   3     2
	for (auto corner : corners)
	{
		data.vertexVec.emplace_back(corner, color);
	}
		
	for (int i = 0; i < 4; ++i)
	{
		data.indexVec.push_back(i);
		data.indexVec.push_back(i + 4);

		data.indexVec.push_back(i);
		data.indexVec.push_back((i + 1) % 4);

		data.indexVec.push_back(i + 4);
		data.indexVec.push_back((i + 1) % 4 + 4);
	}
	
	return data;
}
