//***************************************************************************************
// Model.h by X_Jun(MKXJun) (C) 2018-2020 All Rights Reserved.
// Licensed under the MIT License.
//
// 存放模型数据
// model data storage.
//***************************************************************************************

#ifndef MODEL_H
#define MODEL_H

#include <DirectXCollision.h>
#include <wrl/client.h>

#include "d3dUtil.h"
#include "DXTrace.h"
#include "ObjReader.h"
#include "Geometry.h"

struct ModelPart
{
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	ModelPart() : material(), vertexCount(), indexCount(), indexFormat() {}

	Material material;									// 物体材质
	ComPtr<ID3D11ShaderResourceView> texDiffuse;		// 纹理
	ComPtr<ID3D11ShaderResourceView> texNormalMap;
	ComPtr<ID3D11Buffer> vertexBuffer;					// 顶点缓冲区
	ComPtr<ID3D11Buffer> indexBuffer;					// 索引缓冲区
	UINT vertexCount;									// 顶点字节大小
	UINT indexCount;									// 索引数目	
	DXGI_FORMAT indexFormat;
};

struct Model
{
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	
	Model();
	Model(ID3D11Device* device, const ObjReader& model);
	// 设置缓冲区
	template<typename VertexType, typename IndexType>
	Model(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData);

	template<typename VertexType, typename IndexType>
	Model(ID3D11Device* device, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices);

	Model(ID3D11Device* device, const void* vertices, UINT vertexSize, UINT vertexCount,
		const void* indices, UINT indexCount, DXGI_FORMAT indexFormat);
	//
	// 设置模型
	//

	void SetModel(ID3D11Device* device, const ObjReader& model);

	//
	// 设置网格
	//
	template<typename VertexType, typename IndexType>
	void SetMesh(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData);

	template<typename VertexType, typename IndexType>
	void SetMesh(ID3D11Device* device, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices);

	void SetMesh(ID3D11Device* device, const void* vertices, UINT vertexSize, UINT vertexCount,
		const void* indices, UINT indexCount, DXGI_FORMAT indexFormat);

	//
	// 调试 
	//

	// 设置调试对象名
	// 若模型被重新设置，调试对象名也需要被重新设置
	void SetDebugObjectName(const std::string& name);

	std::vector<ModelPart> modelParts;
	DirectX::BoundingBox boundingBox;
	UINT vertexStride;
};

template<typename VertexType, typename IndexType>
Model::Model(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData)
	: vertexStride()
{
	SetMesh(device, meshData);
}

template<typename VertexType, typename IndexType>
Model::Model(ID3D11Device* device, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices)
	: vertexStride()
{
	SetMesh(device, vertices, indices);
}

template<typename VertexType, typename IndexType>
void Model::SetMesh(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData)
{
	SetMesh(device, meshData.vertexVec, meshData.indexVec);
}

template<typename VertexType, typename IndexType>
void Model::SetMesh(ID3D11Device* device, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices)
{
	static_assert(sizeof(IndexType) == 2 || sizeof(IndexType) == 4, "The size of IndexType must be 2 bytes or 4 bytes!");
	static_assert(std::is_unsigned<IndexType>::value, "IndexType must be unsigned integer!");
	
	SetMesh(device, vertices.data(), sizeof(VertexType),
		static_cast<UINT>(vertices.size()), indices.data(), static_cast<UINT>(indices.size()),
		(sizeof(IndexType) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT));
}

#endif
