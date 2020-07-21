#include "Model.h"

using namespace DirectX;

Model::Model()
	: vertexStride()
{
}

Model::Model(ID3D11Device* device, const ObjReader& model)
	: vertexStride()
{
	SetModel(device, model);
}

Model::Model(ID3D11Device* device, const void* vertices, const UINT vertexSize, const UINT vertexCount,
	const void* indices, const UINT indexCount, const DXGI_FORMAT indexFormat)
	: vertexStride()
{
	SetMesh(device, vertices, vertexSize, vertexCount, indices, indexCount, indexFormat);
}

void Model::SetModel(ID3D11Device* device, const ObjReader& model)
{
	vertexStride = sizeof(VertexPosNormalTex);

	modelParts.resize(model.m_objParts.size());

	// 创建包围盒
	BoundingBox::CreateFromPoints(boundingBox, XMLoadFloat3(&model.m_vMin), XMLoadFloat3(&model.m_vMax));

	for (size_t i = 0; i < model.m_objParts.size(); ++i)
	{
		auto part = model.m_objParts[i];

		modelParts[i].vertexCount = static_cast<UINT>(part.vertices.size());
		// 设置顶点缓冲区描述
		D3D11_BUFFER_DESC vbd;
		ZeroMemory(&vbd, sizeof(vbd));
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = modelParts[i].vertexCount * static_cast<UINT>(sizeof(VertexPosNormalTex));
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		// 新建顶点缓冲区
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = part.vertices.data();
		HR(device->CreateBuffer(&vbd, &InitData, modelParts[i].vertexBuffer.ReleaseAndGetAddressOf()));

		// 设置索引缓冲区描述
		D3D11_BUFFER_DESC ibd;
		ZeroMemory(&ibd, sizeof(ibd));
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		if (modelParts[i].vertexCount > 65535)
		{
			modelParts[i].indexCount = static_cast<UINT>(part.indices32.size());
			modelParts[i].indexFormat = DXGI_FORMAT_R32_UINT;
			ibd.ByteWidth = modelParts[i].indexCount * static_cast<UINT>(sizeof(DWORD));
			InitData.pSysMem = part.indices32.data();

		}
		else
		{
			modelParts[i].indexCount = static_cast<UINT>(part.indices16.size());
			modelParts[i].indexFormat = DXGI_FORMAT_R16_UINT;
			ibd.ByteWidth = modelParts[i].indexCount * static_cast<UINT>(sizeof(WORD));
			InitData.pSysMem = part.indices16.data();
		}
		// 新建索引缓冲区
		HR(device->CreateBuffer(&ibd, &InitData, modelParts[i].indexBuffer.ReleaseAndGetAddressOf()));

		// 创建漫射光对应纹理
		auto& strD = part.texStrDiffuse;
		if (strD.size() > 4)
		{
			if (strD.substr(strD.size() - 3, 3) == L"dds")
			{
				HR(CreateDDSTextureFromFile(device, strD.c_str(), nullptr,
					modelParts[i].texDiffuse.GetAddressOf()));
			}
			else
			{
				HR(CreateWICTextureFromFile(device, strD.c_str(), nullptr,
					modelParts[i].texDiffuse.GetAddressOf()));
			}
		}

		modelParts[i].material = part.material;
	}
}

void Model::SetMesh(ID3D11Device* device, const void* vertices, const UINT vertexSize, const UINT vertexCount, const void* indices, const UINT indexCount, const DXGI_FORMAT indexFormat)
{
	vertexStride = vertexSize;

	modelParts.resize(1);

	modelParts[0].vertexCount = vertexCount;
	modelParts[0].indexCount = indexCount;
	modelParts[0].indexFormat = indexFormat;

	modelParts[0].material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	modelParts[0].material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	modelParts[0].material.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = vertexSize * vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;
	HR(device->CreateBuffer(&vbd, &initData, modelParts[0].vertexBuffer.ReleaseAndGetAddressOf()));

	// 设置索引缓冲区描述
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	if (indexFormat == DXGI_FORMAT_R16_UINT)
	{
		ibd.ByteWidth = indexCount * static_cast<UINT>(sizeof(WORD));
	}
	else
	{
		ibd.ByteWidth = indexCount * static_cast<UINT>(sizeof(DWORD));
	}
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	initData.pSysMem = indices;
	HR(device->CreateBuffer(&ibd, &initData, modelParts[0].indexBuffer.ReleaseAndGetAddressOf()));
}

void Model::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)

	const size_t modelPartSize = modelParts.size();
	for (size_t i = 0; i < modelPartSize; ++i)
	{
		D3D11SetDebugObjectName(modelParts[i].vertexBuffer.Get(), name + ".part[" + std::to_string(i) + "].VertexBuffer");
		D3D11SetDebugObjectName(modelParts[i].indexBuffer.Get(), name + ".part[" + std::to_string(i) + "].IndexBuffer");
	}

#else
	UNREFERENCED_PARAMETER(name);
#endif
}
