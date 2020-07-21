#include "Collision.h"

using namespace DirectX;

Ray::Ray(const XMFLOAT3 origin, FXMVECTOR direction)
	:
	origin(origin),
	direction()
{
	// 射线的direction长度必须为1.0f，误差在1e-5f内
	const XMVECTOR error = XMVectorAbs(XMVector3Length(direction) - XMVectorSplatOne());

	assert(XMVector3Less(error, XMVectorReplicate(1e-5f)));

	XMStoreFloat3(&this->direction, XMVector3Normalize(direction));
}

/*
	一个3D对象的顶点原本是位于局部坐标系的，
	然后经历了世界变换、观察变换、投影变换后，
	会来到NDC空间中，可视物体的深度值(z值)通常会处于0.0到1.0之间。
	而在NDC空间的坐标点还需要经过视口变换，才会来到最终的屏幕坐标系。
	在该坐标系中，坐标原点位于屏幕左上角，x轴向右，y轴向下，
	其中x和y的值指定了绘制在屏幕的位置，z的值则用作深度测试。
	而且从NDC空间到屏幕坐标系的变换只影响x和y的值，对z值不会影响


	而现在我们要做的，就是将选中的2D屏幕点按顺序进行视口逆变换、
	投影逆变换和观察逆变换，让其变换到世界坐标系并以摄像机位置为射线原点，
	构造出一条3D射线，最终才来进行射线与物体的相交。
	在构造屏幕一点的时候，将z值设为0.0即可。z值的变动，
	不会影响构造出来的射线，相当于在射线中前后移动而已
 */

Ray Ray::ScreenToRay(const Camera& camera, const float screenX, const float screenY)
{
	/* 视口逆变换: 将屏幕坐标点从视口变换回NDC坐标系
	 * 将变换矩阵看作一次缩放和一次平移
	 *
	 * NDC到屏幕变换矩阵如下(右侧其次的0001不表):
	 * 
	 * 缩放
	 * [ Width / 2, 0          , 0                   ]
	 * [ 0        , -Height / 2, 0                   ]
	 * [ 0        , 0          , MaxDepth - MinDepth ]
	 *
	 * 平移
	 * [ TopLeftX + Width / 2, TopLeftY + Height / 2, MinDepth ]
	 * 
	 * 逆变换矩阵如下(右侧其次的0001不表):
	 * 
	 * 缩放(我们可以用上面缩放向量的倒数)
	 * [ 2 / Width, 0          , 0                       ]
	 * [ 0        , -2 / Height, 0                       ]
	 * [ 0        , 0          , 1 / MaxDepth - MinDepth ]
	 *
	 * 平移
	 * [ - 2 * TopLeftX / Width - 1 , 2 * TopLeftY / Height + 1, - MinDepth / (MaxDepth - MinDepth) ]
	 *
	 * Vndc = Vscreen * scale + offset
	 */

	const D3D11_VIEWPORT viewPort = camera.GetViewPort();

	XMVECTOR scale = XMVectorSet(viewPort.Width * 0.5f, -viewPort.Height * 0.5f, viewPort.MaxDepth - viewPort.MinDepth, 1.0f);
	// 缩放向量倒数
	scale = XMVectorReciprocal(scale);

	// 先分离多余的-1和+1到D,然后设置offset令其与scale相乘再加上分离的D刚好等于逆变换平移向量
	static const XMVECTORF32 D = { { { -1.0f, 1.0f, 0.0f, 0.0f } } };
	XMVECTOR offset = XMVectorSet(-viewPort.TopLeftX, -viewPort.TopLeftY, -viewPort.MinDepth, 0.0f);
	// 先乘后加
	offset = XMVectorMultiplyAdd(scale, offset, D.v);
	// Vscreen
	const XMVECTOR vScreen = XMVectorSet(screenX, screenY, 0.0f, 1.0f);
	// Vndc
	XMVECTOR target = XMVectorMultiplyAdd(vScreen, scale, offset);
	
	// 投影逆变换和观察逆变换: 从NDC坐标系变换回世界坐标系

	// Proj^-1 * View^-1 = (View * Proj)^-1
	XMMATRIX transform = XMMatrixMultiply(camera.GetViewMatrix(), camera.GetProjMatrix());
	transform = XMMatrixInverse(nullptr, transform);
	// Vworld = Vndc * Proj^-1 * View^-1
	// 把向量转为矩阵然后乘上矩阵再返回向量
	target = XMVector3TransformCoord(target, transform);

	// 求出射线
	return { camera.GetPositionFloat3(), XMVector3Normalize(target - camera.GetPositionVector()) };
}

bool Ray::Hit(const BoundingBox& box, float* pOutDist, const float maxDist) const
{

	float dist;
	const bool res = box.Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), dist);
	if (pOutDist)
		*pOutDist = dist;
	return dist > maxDist ? false : res;
}

bool Ray::Hit(const BoundingOrientedBox& box, float* pOutDist, const float maxDist) const
{
	float dist;
	const bool res = box.Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), dist);
	if (pOutDist)
		*pOutDist = dist;
	return dist > maxDist ? false : res;
}

bool Ray::Hit(const BoundingSphere& sphere, float* pOutDist, const float maxDist) const
{
	float dist;
	const bool res = sphere.Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), dist);
	if (pOutDist)
		*pOutDist = dist;
	return dist > maxDist ? false : res;
}

bool XM_CALLCONV Ray::Hit(FXMVECTOR vertex0, FXMVECTOR vertex1, FXMVECTOR vertex2, float* pOutDist, const float maxDist) const
{
	float dist;
	const bool res = TriangleTests::Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), vertex0, vertex1, vertex2, dist);
	if (pOutDist)
		*pOutDist = dist;
	return dist > maxDist ? false : res;
}

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
	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, proj);
	// 将视锥体从局部坐标系变换到世界坐标系中
	frustum.Transform(frustum, XMMatrixInverse(nullptr, view));

	BoundingOrientedBox localOrientedBox;
	BoundingOrientedBox orientedBox;
	BoundingOrientedBox::CreateFromBoundingBox(localOrientedBox, localBox);

	std::vector<XMMATRIX> acceptedData;
	for (const XMMATRIX& matrix : matrices)
	{
		// 将有向包围盒从局部坐标系变换到世界坐标系中
		localOrientedBox.Transform(orientedBox, matrix);
		// 相交检测
		if (frustum.Intersects(orientedBox))
			acceptedData.push_back(matrix);
	}

	return acceptedData;
}

std::vector<XMMATRIX> XM_CALLCONV Collision::FrustumCulling2(
	const std::vector<XMMATRIX>& matrices,const BoundingBox& localBox, FXMMATRIX view, CXMMATRIX proj)
{
	BoundingFrustum frustum;
	BoundingFrustum localFrustum;
	BoundingFrustum::CreateFromMatrix(frustum, proj);
	const XMMATRIX invView = XMMatrixInverse(nullptr, view);

	std::vector<XMMATRIX> acceptedData;
	for (const XMMATRIX& matrix : matrices)
	{
		// 将视锥体从观察坐标系(或局部坐标系)变换到物体所在的局部坐标系中
		frustum.Transform(localFrustum, invView * XMMatrixInverse(nullptr, matrix));
		// 相交检测
		if (localFrustum.Intersects(localBox))
			acceptedData.push_back(matrix);
	}

	return acceptedData;
}

std::vector<XMMATRIX> XM_CALLCONV Collision::FrustumCulling3(
	const std::vector<XMMATRIX>& matrices,const BoundingBox& localBox, FXMMATRIX view, CXMMATRIX proj)
{
	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, proj);

	BoundingOrientedBox localOrientedBox;
	BoundingOrientedBox orientedBox;
	BoundingOrientedBox::CreateFromBoundingBox(localOrientedBox, localBox);

	std::vector<XMMATRIX> acceptedData;
	for (const XMMATRIX& matrix : matrices)
	{
		// 将有向包围盒从局部坐标系变换到视锥体所在的局部坐标系(观察坐标系)中
		localOrientedBox.Transform(orientedBox, matrix * view);
		// 相交检测
		if (frustum.Intersects(orientedBox))
			acceptedData.push_back(matrix);
	}

	return acceptedData;
}

/*
	由投影矩阵构造出来的视锥体包围盒也位于自身局部坐标系中，
	而观察矩阵实质上是从世界矩阵变换到视锥体所处的局部坐标系中。
	因此，我们可以使用观察矩阵的逆矩阵，将视锥体包围盒也变换到世界空间中。
	这样就好似物体与视锥体都位于世界空间中，可以进行碰撞检测了
 */
std::vector<BasicTransform> XM_CALLCONV Collision::FrustumCulling(
	const std::vector<BasicTransform>& transforms, const BoundingBox& localBox, FXMMATRIX view, CXMMATRIX proj)
{
	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, proj);

	BoundingOrientedBox localOrientedBox;
	BoundingOrientedBox orientedBox;
	BoundingOrientedBox::CreateFromBoundingBox(localOrientedBox, localBox);

	std::vector<BasicTransform> acceptedData;
	for (const BasicTransform& transform : transforms)
	{
		// 将有向包围盒从局部坐标系变换到视锥体所在的局部坐标系(观察坐标系)中
		localOrientedBox.Transform(orientedBox, transform.GetLocalToWorldMatrix() * view);
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
std::vector<BasicTransform> XM_CALLCONV Collision::FrustumCulling2(
	const std::vector<BasicTransform>& transforms, const BoundingBox& localBox, FXMMATRIX view, CXMMATRIX proj)
{
	BoundingFrustum frustum;
	BoundingFrustum localFrustum;
	BoundingFrustum::CreateFromMatrix(frustum, proj);
	const XMMATRIX invView = XMMatrixInverse(nullptr, view);

	std::vector<BasicTransform> acceptedData;
	for (const BasicTransform& transform : transforms)
	{
		// 将视锥体从观察坐标系(或局部坐标系)变换到物体所在的局部坐标系中
		frustum.Transform(localFrustum, invView * XMMatrixInverse(nullptr, transform.GetLocalToWorldMatrix()));
		// 相交检测
		if (localFrustum.Intersects(localBox))
			acceptedData.push_back(transform);
	}

	return acceptedData;
}

/*
	先将物体从局部坐标系搬移到世界坐标系，
	然后再用观察矩阵将其搬移到视锥体自身的局部坐标系来与视锥体进行碰撞检测
 */
std::vector<BasicTransform> XM_CALLCONV Collision::FrustumCulling3(
	const std::vector<BasicTransform>& transforms, const BoundingBox& localBox, FXMMATRIX view, CXMMATRIX proj)
{
	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, proj);

	BoundingOrientedBox localOrientedBox;
	BoundingOrientedBox orientedBox;
	BoundingOrientedBox::CreateFromBoundingBox(localOrientedBox, localBox);

	std::vector<BasicTransform> acceptedData;
	for (const BasicTransform& transform : transforms)
	{
		// 将有向包围盒从局部坐标系变换到视锥体所在的局部坐标系(观察坐标系)中
		localOrientedBox.Transform(orientedBox, transform.GetLocalToWorldMatrix() * view);
		// 相交检测
		if (frustum.Intersects(orientedBox))
			acceptedData.push_back(transform);
	}

	return acceptedData;
}

Collision::WireFrameData Collision::CreateFromCorners(const XMFLOAT3(&corners)[8], const XMFLOAT4& color)
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
