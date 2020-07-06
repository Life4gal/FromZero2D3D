//***************************************************************************************
// Geometry.h by X_Jun(MKXJun) (C) 2018-2020 All Rights Reserved.
// Licensed under the MIT License.
//
// 生成常见的几何体网格模型
// Generate common geometry meshes.
//***************************************************************************************
//
// Life4Gal 进行了大量改动, 总体使用方式无差异
// 

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <vector>
#include <string>
#include <map>
#include <functional>
#include "Vertex.h"

namespace Geometry
{
	// 网格数据
	template<class VertexType = VertexPosNormalTex, class IndexType = DWORD>
	struct MeshData
	{
		std::vector<VertexType> vertexVec;	// 顶点数组
		std::vector<IndexType> indexVec;	// 索引数组

		MeshData()
		{
			// 需检验索引类型合法性
			static_assert(sizeof(IndexType) == 2 || sizeof(IndexType) == 4, "The size of IndexType must be 2 bytes or 4 bytes!");
			static_assert(std::is_unsigned<IndexType>::value, "IndexType must be unsigned integer!");
		}
	};

	// 创建球体网格数据，levels和slices越大，精度越高。
	template<class VertexType = VertexPosNormalTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> CreateSphere(float radius = 1.0f, UINT levels = 20, UINT slices = 20,
		const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	// 创建立方体网格数据,参数是俯视角看,实际图形为绕Y轴旋转90度后状态
	template<class VertexType = VertexPosNormalTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> CreateBox(float width = 2.0f, float length = 2.0f, float height = 2.0f,
		const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	// 创建圆柱体网格数据，slices越大，精度越高。
	template<class VertexType = VertexPosNormalTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> CreateCylinder(float radius = 1.0f, float height = 2.0f, UINT slices = 20,
		const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	// 创建只有圆柱体侧面的网格数据，slices越大，精度越高
	template<class VertexType = VertexPosNormalTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> CreateCylinderNoCap(float radius = 1.0f, float height = 2.0f, UINT slices = 20,
		const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	// 创建圆锥体网格数据，slices越大，精度越高。
	template<class VertexType = VertexPosNormalTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> CreateCone(float radius = 1.0f, float height = 2.0f, UINT slices = 20,
		const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	// 创建只有圆锥体侧面网格数据，slices越大，精度越高。
	template<class VertexType = VertexPosNormalTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> CreateConeNoCap(float radius = 1.0f, float height = 2.0f, UINT slices = 20,
		const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	// 创建一个指定NDC屏幕区域的面(默认全屏)
	template<class VertexType = VertexPosTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> Create2DShow(const DirectX::XMFLOAT2& center, const DirectX::XMFLOAT2& scale, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	template<class VertexType = VertexPosTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> Create2DShow(float centerX = 0.0f, float centerY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	// 创建一个平面
	template<class VertexType = VertexPosNormalTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> CreatePlane(const DirectX::XMFLOAT2& planeSize, 
		const DirectX::XMFLOAT2& maxTexCoord = { 1.0f, 1.0f }, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	template<class VertexType = VertexPosNormalTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> CreatePlane(float width = 10.0f, float depth = 10.0f, float texU = 1.0f, float texV = 1.0f,
		const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	// 创建一个地形
	template<class VertexType = VertexPosNormalTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> CreateTerrain(const DirectX::XMFLOAT2& terrainSize,
		const DirectX::XMUINT2& slices = { 10, 10 }, const DirectX::XMFLOAT2 & maxTexCoord = { 1.0f, 1.0f },
		const std::function<float(float, float)>& heightFunc = [](float x, float z) { return 0.0f; },
		const std::function<DirectX::XMFLOAT3(float, float)>& normalFunc = [](float x, float z) { return DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f); },
		const std::function<DirectX::XMFLOAT4(float, float)>& colorFunc = [](float x, float z) { return DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); });
	template<class VertexType = VertexPosNormalTex, class IndexType = DWORD>
	MeshData<VertexType, IndexType> CreateTerrain(float width = 10.0f, float depth = 10.0f,
		UINT slicesX = 10, UINT slicesZ = 10, float texU = 1.0f, float texV = 1.0f,
		const std::function<float(float, float)>& heightFunc = [](float x, float z) { return 0.0f; },
		const std::function<DirectX::XMFLOAT3(float, float)>& normalFunc = [](float x, float z) { return DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f); },
		const std::function<DirectX::XMFLOAT4(float, float)>& colorFunc = [](float x, float z) { return DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); });
}

namespace Geometry
{
	namespace Internal
	{
		//
		// 以下结构体和函数仅供内部实现使用
		//

		struct VertexData
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT3 normal;
			DirectX::XMFLOAT4 tangent;
			DirectX::XMFLOAT4 color;
			DirectX::XMFLOAT2 tex;
		};

		// 根据目标顶点类型选择性将数据插入
		template<class VertexType>
		void InsertVertexElement(VertexType& vertexDst, const VertexData& vertexSrc)
		{
			static std::string semanticName;
			static const std::map<std::string, std::pair<size_t, size_t>> semanticSizeMap = {
				{"POSITION", std::pair<size_t, size_t>(0, 12)},
				{"NORMAL", std::pair<size_t, size_t>(12, 24)},
				{"TANGENT", std::pair<size_t, size_t>(24, 40)},
				{"COLOR", std::pair<size_t, size_t>(40, 56)},
				{"TEXCOORD", std::pair<size_t, size_t>(56, 64)}
			};

			for (size_t i = 0; i < ARRAYSIZE(VertexType::inputLayout); i++)
			{
				semanticName = VertexType::inputLayout[i].SemanticName;
				const auto& range = semanticSizeMap.at(semanticName);
				memcpy_s(reinterpret_cast<char*>(&vertexDst) + VertexType::inputLayout[i].AlignedByteOffset,
					range.second - range.first,
					reinterpret_cast<const char*>(&vertexSrc) + range.first,
					range.second - range.first);
			}
		}
	}
	
	//
	// 几何体方法的实现
	//

	/*
		levels决定上下分多少层，slices决定一个水平圆切面的顶点数目。
		levels和slices越高，生成的顶点数、索引数都会越多。
	 */
	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreateSphere(float radius, UINT levels, UINT slices, const DirectX::XMFLOAT4 & color)
	{
		using namespace DirectX;
		
		MeshData<VertexType, IndexType> meshData;
		// 对于每个圆面,有 slices + 1(起点和终点重合) 个顶点, levels - 1 个平面可以切割出 levels 层, 再加上是上下两个顶点
		UINT vertexCount = 2 + (levels - 1) * (slices + 1);
		// 每个点连出六条线(算上了上下两个顶点额外射出的线), 共有 levels - 1 个平面
		UINT indexCount = 6 * (levels - 1) * slices;
		meshData.vertexVec.resize(vertexCount);
		meshData.indexVec.resize(indexCount);

		// YOZ平面半个圆面 PI 分成 levels 个扇形, 可以求出高度 Y
		float per_phi = XM_PI / levels;
		// XO`Z(根据高度Y不同, O`不同)圆面 2PI 分成 slices 个扇形, 配合高度 Y 求出 X, Z
		float per_theta = XM_2PI / slices;

		// 先把球体顶端的额外顶点加入
		Internal::VertexData vertexData = { XMFLOAT3(0.0f, radius, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 0.0f) };
		// 顶点下标
		IndexType vIndex = 0;
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		// 自顶向下一层一层遍历, 因为有 levels - 1 个平面, 所以从 1 开始
		for (UINT i = 1; i < levels; ++i)
		{
			// 获取角度以求得高度 Y
			const float phi = per_phi * i;
			// 这里用 cos(phi) 求高度, 因为角度从顶部向底部递增
			const float y = radius * cosf(phi);
			// 当前高度对应圆面的半径
			const float curr_radius = radius * sinf(phi);
			// 需要slices + 1个顶点是因为 起点和终点需为同一点，但纹理坐标值不一致
			for (UINT j = 0; j <= slices; ++j)
			{
				// 先 radius * sinf(phi) 获取当前高度圆面的 半径 , 然后再根据圆面的角度求得 X, Z
				const float theta = per_theta * j;
				const float x = curr_radius * cosf(theta);
				const float z = curr_radius * sinf(theta);
				// 计算出局部坐标、法向量、切向量和纹理坐标
				XMFLOAT3 pos{ x, y, z };
				XMFLOAT3 normal{};
				// 球面法向量 = (点向量 - 球心坐标) ^ 单位向量化 , 其中球心坐标为(0, 0, 0)
				XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&pos)));
				// 纹理坐标 x = j / slices , y = i / levels
				vertexData = { pos, normal, XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f), color, XMFLOAT2(theta / XM_2PI, phi / XM_PI) };
				Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);
			}
		}

		// 先把球体底端的额外顶点加入
		vertexData = { XMFLOAT3(0.0f, -radius, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),
			XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 1.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);


		// 放入索引
		// 索引下标
		IndexType iIndex = 0;
		for (UINT j = 1; j <= slices; ++j)
		{
			meshData.indexVec[iIndex++] = 0;
			meshData.indexVec[iIndex++] = j % (slices + 1) + 1;
			meshData.indexVec[iIndex++] = j;
		}

		for (UINT i = 1; i < levels - 1; ++i)
		{
			for (UINT j = 1; j <= slices; ++j)
			{
				meshData.indexVec[iIndex++] = (i - 1) * (slices + 1) + j;
				meshData.indexVec[iIndex++] = (i - 1) * (slices + 1) + j % (slices + 1) + 1;
				meshData.indexVec[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;

				meshData.indexVec[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;
				meshData.indexVec[iIndex++] = i * (slices + 1) + j;
				meshData.indexVec[iIndex++] = (i - 1) * (slices + 1) + j;
			}
		}

		// 逐渐放入索引
		if (levels > 1)
		{
			for (UINT j = 1; j <= slices; ++j)
			{
				meshData.indexVec[iIndex++] = (levels - 2) * (slices + 1) + j;
				meshData.indexVec[iIndex++] = (levels - 2) * (slices + 1) + j % (slices + 1) + 1;
				meshData.indexVec[iIndex++] = (levels - 1) * (slices + 1) + 1;
			}
		}

		return meshData;
	}

	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreateBox(float width, float length, float height, const DirectX::XMFLOAT4 & color)
	{
		using namespace DirectX;

		MeshData<VertexType, IndexType> meshData;
		meshData.vertexVec.resize(24);

		Internal::VertexData vertexDataArr[24];
		const float l2 = length / 2;
		const float h2 = height / 2;
		const float w2 = width / 2;

		// 右面(+X面)
		vertexDataArr[0].pos = XMFLOAT3(l2, -h2, -w2);
		vertexDataArr[1].pos = XMFLOAT3(l2, h2, -w2);
		vertexDataArr[2].pos = XMFLOAT3(l2, h2, w2);
		vertexDataArr[3].pos = XMFLOAT3(l2, -h2, w2);
		// 左面(-X面)
		vertexDataArr[4].pos = XMFLOAT3(-l2, -h2, w2);
		vertexDataArr[5].pos = XMFLOAT3(-l2, h2, w2);
		vertexDataArr[6].pos = XMFLOAT3(-l2, h2, -w2);
		vertexDataArr[7].pos = XMFLOAT3(-l2, -h2, -w2);
		// 顶面(+Y面)
		vertexDataArr[8].pos = XMFLOAT3(-l2, h2, -w2);
		vertexDataArr[9].pos = XMFLOAT3(-l2, h2, w2);
		vertexDataArr[10].pos = XMFLOAT3(l2, h2, w2);
		vertexDataArr[11].pos = XMFLOAT3(l2, h2, -w2);
		// 底面(-Y面)
		vertexDataArr[12].pos = XMFLOAT3(l2, -h2, -w2);
		vertexDataArr[13].pos = XMFLOAT3(l2, -h2, w2);
		vertexDataArr[14].pos = XMFLOAT3(-l2, -h2, w2);
		vertexDataArr[15].pos = XMFLOAT3(-l2, -h2, -w2);
		// 背面(+Z面)
		vertexDataArr[16].pos = XMFLOAT3(l2, -h2, w2);
		vertexDataArr[17].pos = XMFLOAT3(l2, h2, w2);
		vertexDataArr[18].pos = XMFLOAT3(-l2, h2, w2);
		vertexDataArr[19].pos = XMFLOAT3(-l2, -h2, w2);
		// 正面(-Z面)
		vertexDataArr[20].pos = XMFLOAT3(-l2, -h2, -w2);
		vertexDataArr[21].pos = XMFLOAT3(-l2, h2, -w2);
		vertexDataArr[22].pos = XMFLOAT3(l2, h2, -w2);
		vertexDataArr[23].pos = XMFLOAT3(l2, -h2, -w2);

		for (UINT i = 0; i < 4; ++i)
		{
			// 右面(+X面)
			vertexDataArr[i].normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
			vertexDataArr[i].tangent = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
			vertexDataArr[i].color = color;
			// 左面(-X面)
			vertexDataArr[i + 4].normal = XMFLOAT3(-1.0f, 0.0f, 0.0f);
			vertexDataArr[i + 4].tangent = XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f);
			vertexDataArr[i + 4].color = color;
			// 顶面(+Y面)
			vertexDataArr[i + 8].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			vertexDataArr[i + 8].tangent = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
			vertexDataArr[i + 8].color = color;
			// 底面(-Y面)
			vertexDataArr[i + 12].normal = XMFLOAT3(0.0f, -1.0f, 0.0f);
			vertexDataArr[i + 12].tangent = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
			vertexDataArr[i + 12].color = color;
			// 背面(+Z面)
			vertexDataArr[i + 16].normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
			vertexDataArr[i + 16].tangent = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
			vertexDataArr[i + 16].color = color;
			// 正面(-Z面)
			vertexDataArr[i + 20].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
			vertexDataArr[i + 20].tangent = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
			vertexDataArr[i + 20].color = color;
		}

		for (UINT i = 0; i < 6; ++i)
		{
			vertexDataArr[i * 4].tex = XMFLOAT2(0.0f, 1.0f);
			vertexDataArr[i * 4 + 1].tex = XMFLOAT2(0.0f, 0.0f);
			vertexDataArr[i * 4 + 2].tex = XMFLOAT2(1.0f, 0.0f);
			vertexDataArr[i * 4 + 3].tex = XMFLOAT2(1.0f, 1.0f);
		}

		for (UINT i = 0; i < 24; ++i)
		{
			Internal::InsertVertexElement(meshData.vertexVec[i], vertexDataArr[i]);
		}

		meshData.indexVec = {
			0, 1, 2, 2, 3, 0,		// 右面(+X面)
			4, 5, 6, 6, 7, 4,		// 左面(-X面)
			8, 9, 10, 10, 11, 8,	// 顶面(+Y面)
			12, 13, 14, 14, 15, 12,	// 底面(-Y面)
			16, 17, 18, 18, 19, 16, // 背面(+Z面)
			20, 21, 22, 22, 23, 20	// 正面(-Z面)
		};

		return meshData;
	}

	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreateCylinder(float radius, float height, UINT slices, const DirectX::XMFLOAT4 & color)
	{
		using namespace DirectX;

		MeshData<VertexType, IndexType>  meshData = CreateCylinderNoCap<VertexType, IndexType>(radius, height, slices, color);
		UINT vertexCount = 4 * (slices + 1) + 2;
		UINT indexCount = 12 * slices;
		meshData.vertexVec.resize(vertexCount);
		meshData.indexVec.resize(indexCount);

		float h2 = height / 2;
		float per_theta = XM_2PI / slices;

		IndexType vIndex = 2 * (slices + 1), iIndex = 6 * slices;
		IndexType offset = 2 * (slices + 1);
		Internal::VertexData vertexData{};

		// 放入顶端圆心
		vertexData = { XMFLOAT3(0.0f, h2, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.5f, 0.5f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		// 放入顶端圆上各点
		for (UINT i = 0; i <= slices; ++i)
		{
			float theta = i * per_theta;
			vertexData = { XMFLOAT3(radius * cosf(theta), h2, radius * sinf(theta)), XMFLOAT3(0.0f, 1.0f, 0.0f),
				XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f) };
			Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);
		}

		// 放入底端圆心
		vertexData = { XMFLOAT3(0.0f, -h2, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),
			XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.5f, 0.5f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		// 放入底部圆上各点
		for (UINT i = 0; i <= slices; ++i)
		{
			float theta = i * per_theta;
			vertexData = { XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta)), XMFLOAT3(0.0f, -1.0f, 0.0f),
				XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f) };
			Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);
		}

		// 放入顶部三角形索引
		for (UINT i = 1; i <= slices; ++i)
		{
			meshData.indexVec[iIndex++] = offset;
			meshData.indexVec[iIndex++] = offset + i % (slices + 1) + 1;
			meshData.indexVec[iIndex++] = offset + i;
		}

		// 放入底部三角形索引
		offset += slices + 2;
		for (UINT i = 1; i <= slices; ++i)
		{
			meshData.indexVec[iIndex++] = offset;
			meshData.indexVec[iIndex++] = offset + i;
			meshData.indexVec[iIndex++] = offset + i % (slices + 1) + 1;
		}

		return meshData;
	}

	/*
		无上下圆面的圆柱体
	 */
	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreateCylinderNoCap(float radius, float height, UINT slices, const DirectX::XMFLOAT4 & color)
	{
		using namespace DirectX;

		MeshData<VertexType, IndexType> meshData;
		// 上下两个圆面,每个圆面有 slices + 1(圆心点) 个顶点
		UINT vertexCount = 2 * (slices + 1);
		// 每个点连出五条线,额外的圆心点对每个点都连接一条线,即总计六倍的点数
		UINT indexCount = 6 * slices;
		meshData.vertexVec.resize(vertexCount);
		meshData.indexVec.resize(indexCount);

		// 原点在图形中心,上下各占高度一半
		const float h2 = height / 2;
		// 一个圆面 2PI 分成 slices 个扇形
		const float per_theta = XM_2PI / slices;
		
		//储存顶点数据
		Internal::VertexData vertexData{};

		// 放入侧面顶端点
		for (UINT i = 0; i <= slices; ++i)
		{
			// 从 0°开始,每次增加一个扇形
			/*
				pos(位置):
					对于一个平面圆, 朝右表示 x 方向, 朝上表示 z 方向, x = r * cos(theta) , z = r * sin(theta) 
					以摄像机角度看图形, 朝上表示 y 方向, y = h / 2
				normal(法线):

				tangent(切线):

				tex(纹理坐标):
				
			 */
			const float theta = i * per_theta;
			vertexData = { XMFLOAT3(radius * cosf(theta), h2, radius * sinf(theta)), XMFLOAT3(cosf(theta), 0.0f, sinf(theta)),
				XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f), color, XMFLOAT2(theta / XM_2PI, 0.0f) };
			Internal::InsertVertexElement(meshData.vertexVec[i], vertexData);
		}

		// 放入侧面底端点
		for (UINT i = 0; i <= slices; ++i)
		{
			const float theta = i * per_theta;
			vertexData = { XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta)), XMFLOAT3(cosf(theta), 0.0f, sinf(theta)),
				XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f), color, XMFLOAT2(theta / XM_2PI, 1.0f) };
			UINT vIndex = (slices + 1) + i;
			Internal::InsertVertexElement(meshData.vertexVec[vIndex], vertexData);
		}

		// 放入索引
		UINT iIndex = 0;

		for (UINT i = 0; i < slices; ++i)
		{
			meshData.indexVec[iIndex++] = i;
			meshData.indexVec[iIndex++] = i + 1;
			meshData.indexVec[iIndex++] = (slices + 1) + i + 1;

			meshData.indexVec[iIndex++] = (slices + 1) + i + 1;
			meshData.indexVec[iIndex++] = (slices + 1) + i;
			meshData.indexVec[iIndex++] = i;
		}

		return meshData;
	}

	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreateCone(float radius, float height, UINT slices, const DirectX::XMFLOAT4& color)
	{
		using namespace DirectX;
		auto meshData = CreateConeNoCap<VertexType, IndexType>(radius, height, slices, color);

		UINT vertexCount = 3 * slices + 1;
		UINT indexCount = 6 * slices;
		meshData.vertexVec.resize(vertexCount);
		meshData.indexVec.resize(indexCount);

		const float h2 = height / 2;
		const float per_theta = XM_2PI / slices;
		UINT iIndex = 3 * slices;
		UINT vIndex = 2 * slices;
		Internal::VertexData vertexData{};

		// 放入圆锥底面顶点
		for (UINT i = 0; i < slices; ++i)
		{
			const float theta = i * per_theta;
			vertexData = { XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta)), XMFLOAT3(0.0f, -1.0f, 0.0f),
				XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f) };
			Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);
		}
		// 放入圆锥底面圆心
		vertexData = { XMFLOAT3(0.0f, -h2, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),
				XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.5f, 0.5f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		// 放入索引
		const UINT offset = 2 * slices;
		for (UINT i = 0; i < slices; ++i)
		{
			meshData.indexVec[iIndex++] = offset + slices;
			meshData.indexVec[iIndex++] = offset + i % slices;
			meshData.indexVec[iIndex++] = offset + (i + 1) % slices;
		}

		return meshData;
	}

	/*
		不包含底部的圆锥体
	 */
	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreateConeNoCap(float radius, float height, UINT slices, const DirectX::XMFLOAT4& color)
	{
		using namespace DirectX;

		MeshData<VertexType, IndexType> meshData;
		UINT vertexCount = 2 * slices;
		UINT indexCount = 3 * slices;
		meshData.vertexVec.resize(vertexCount);
		meshData.indexVec.resize(indexCount);

		const float h2 = height / 2;
		float theta;
		const float per_theta = XM_2PI / slices;
		const float len = sqrtf(height * height + radius * radius);
		UINT iIndex = 0;
		UINT vIndex = 0;
		Internal::VertexData vertexData{};

		// 放入圆锥尖端顶点(每个顶点位置相同，但包含不同的法向量和切线向量)
		for (UINT i = 0; i < slices; ++i)
		{
			theta = i * per_theta + per_theta / 2;
			vertexData = { XMFLOAT3(0.0f, h2, 0.0f), XMFLOAT3(radius * cosf(theta) / len, height / len, radius * sinf(theta) / len),
				XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f), color, XMFLOAT2(0.5f, 0.5f) };
			Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);
		}

		// 放入圆锥侧面底部顶点
		for (UINT i = 0; i < slices; ++i)
		{
			theta = i * per_theta;
			vertexData = { XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta)), XMFLOAT3(radius * cosf(theta) / len, height / len, radius * sinf(theta) / len),
				XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f), color, XMFLOAT2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f) };
			Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);
		}

		// 放入索引
		for (UINT i = 0; i < slices; ++i)
		{
			meshData.indexVec[iIndex++] = i;
			meshData.indexVec[iIndex++] = slices + (i + 1) % slices;
			meshData.indexVec[iIndex++] = slices + i % slices;
		}

		return meshData;
	}

	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> Create2DShow(const DirectX::XMFLOAT2& center, const DirectX::XMFLOAT2 & scale, const DirectX::XMFLOAT4 & color)
	{
		return Create2DShow<VertexType, IndexType>(center.x, center.y, scale.x, scale.y, color);
	}

	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> Create2DShow(float centerX, float centerY, float scaleX, float scaleY, const DirectX::XMFLOAT4 & color)
	{
		using namespace DirectX;

		MeshData<VertexType, IndexType> meshData;
		meshData.vertexVec.resize(4);

		UINT vIndex = 0;

		Internal::VertexData vertexData = {
			XMFLOAT3(centerX - scaleX, centerY - scaleY, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 1.0f)
		};
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(centerX - scaleX, centerY + scaleY, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(centerX + scaleX, centerY + scaleY, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(1.0f, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(centerX + scaleX, centerY - scaleY, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(1.0f, 1.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		meshData.indexVec = { 0, 1, 2, 2, 3, 0 };
		return meshData;
	}

	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreatePlane(const DirectX::XMFLOAT2 & planeSize,
	                                            const DirectX::XMFLOAT2 & maxTexCoord, const DirectX::XMFLOAT4 & color)
	{
		return CreatePlane<VertexType, IndexType>(planeSize.x, planeSize.y, maxTexCoord.x, maxTexCoord.y, color);
	}

	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreatePlane(float width, float depth, float texU, float texV, const DirectX::XMFLOAT4 & color)
	{
		using namespace DirectX;

		MeshData<VertexType, IndexType> meshData;
		meshData.vertexVec.resize(4);

		UINT vIndex = 0;

		Internal::VertexData vertexData = {
			XMFLOAT3(-width / 2, 0.0f, -depth / 2), XMFLOAT3(0.0f, 1.0f, 0.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, texV)
		};
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(-width / 2, 0.0f, depth / 2), XMFLOAT3(0.0f, 1.0f, 0.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(width / 2, 0.0f, depth / 2), XMFLOAT3(0.0f, 1.0f, 0.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(texU, 0.0f) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		vertexData = { XMFLOAT3(width / 2, 0.0f, -depth / 2), XMFLOAT3(0.0f, 1.0f, 0.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(texU, texV) };
		Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);

		meshData.indexVec = { 0, 1, 2, 2, 3, 0 };
		return meshData;
	}
	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreateTerrain(const DirectX::XMFLOAT2& terrainSize, const DirectX::XMUINT2& slices,
		const DirectX::XMFLOAT2& maxTexCoord, const std::function<float(float, float)>& heightFunc,
		const std::function<DirectX::XMFLOAT3(float, float)>& normalFunc, 
		const std::function<DirectX::XMFLOAT4(float, float)>& colorFunc)
	{
		return CreateTerrain<VertexType, IndexType>(terrainSize.x, terrainSize.y, slices.x, slices.y,
			maxTexCoord.x, maxTexCoord.y, heightFunc, normalFunc, colorFunc);
	}

	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreateTerrain(float width, float depth, UINT slicesX, UINT slicesZ,
		float texU, float texV, const std::function<float(float, float)>& heightFunc,
		const std::function<DirectX::XMFLOAT3(float, float)>& normalFunc,
		const std::function<DirectX::XMFLOAT4(float, float)>& colorFunc)
	{
		using namespace DirectX;

		MeshData<VertexType, IndexType> meshData;
		UINT vertexCount = (slicesX + 1) * (slicesZ + 1);
		UINT indexCount = 6 * slicesX * slicesZ;
		meshData.vertexVec.resize(vertexCount);
		meshData.indexVec.resize(indexCount);

		UINT vIndex = 0;
		UINT iIndex = 0;

		const float sliceWidth = width / slicesX;
		const float sliceDepth = depth / slicesZ;
		const float leftBottomX = -width / 2;
		const float leftBottomZ = -depth / 2;
		const float sliceTexWidth = texU / slicesX;
		const float sliceTexDepth = texV / slicesZ;

		XMFLOAT3 normal;
		XMFLOAT4 tangent;
		// 创建网格顶点
		//  __ __
		// | /| /|
		// |/_|/_|
		// | /| /| 
		// |/_|/_|
		for (UINT z = 0; z <= slicesZ; ++z)
		{
			for (UINT x = 0; x <= slicesX; ++x)
			{
				const float posZ = leftBottomZ + z * sliceDepth;
				const float posX = leftBottomX + x * sliceWidth;
				// 计算法向量并归一化
				normal = normalFunc(posX, posZ);
				XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&normal)));
				// 计算法平面与z=posZ平面构成的直线单位切向量，维持w分量为1.0f
				XMStoreFloat4(&tangent, XMVector3Normalize(XMVectorSet(normal.y, -normal.x, 0.0f, 0.0f)) + g_XMIdentityR3);

				Internal::VertexData vertexData = {
					XMFLOAT3(posX, heightFunc(posX, posZ), posZ),
					normal, tangent, colorFunc(posX, posZ), XMFLOAT2(x * sliceTexWidth, texV - z * sliceTexDepth)
				};
				Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);
			}
		}
		// 放入索引
		for (UINT i = 0; i < slicesZ; ++i)
		{
			for (UINT j = 0; j < slicesX; ++j)
			{
				meshData.indexVec[iIndex++] = i * (slicesX + 1) + j;
				meshData.indexVec[iIndex++] = (i + 1) * (slicesX + 1) + j;
				meshData.indexVec[iIndex++] = (i + 1) * (slicesX + 1) + j + 1;

				meshData.indexVec[iIndex++] = (i + 1) * (slicesX + 1) + j + 1;
				meshData.indexVec[iIndex++] = i * (slicesX + 1) + j + 1;
				meshData.indexVec[iIndex++] = i * (slicesX + 1) + j;
			}
		}

		return meshData;
	}
}


#endif


