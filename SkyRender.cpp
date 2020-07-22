#include "SkyRender.h"

#pragma warning(disable: 26812)

using namespace DirectX;

HRESULT SkyRender::InitResource(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& cubeMapFilename, const float skySphereRadius, const bool generateMips)
{
	// 防止重复初始化造成内存泄漏
	m_pIndexBuffer.Reset();
	m_pVertexBuffer.Reset();
	m_pTextureCubeSRV.Reset();

	HRESULT hr;
	// 天空盒纹理加载
	if (cubeMapFilename.substr(cubeMapFilename.size() - 3) == L"dds")
	{
		hr = CreateDDSTextureFromFile(device,
			generateMips ? deviceContext : nullptr,
			cubeMapFilename.c_str(),
			nullptr,
			m_pTextureCubeSRV.GetAddressOf());
	}
	else
	{
		hr = CreateWICTexture2DCubeFromFile(device,
			deviceContext,
			cubeMapFilename,
			nullptr,
			m_pTextureCubeSRV.GetAddressOf(),
			generateMips);
	}

	if (FAILED(hr))
		return hr;

	return InitResource(device, skySphereRadius);
}

HRESULT SkyRender::InitResource(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::vector<std::wstring>& cubeMapFilename, const float skySphereRadius, const bool generateMips)
{
	// 防止重复初始化造成内存泄漏
	m_pIndexBuffer.Reset();
	m_pVertexBuffer.Reset();
	m_pTextureCubeSRV.Reset();

	// 天空盒纹理加载
	const HRESULT hr = CreateWICTexture2DCubeFromFile(device,
	                                                  deviceContext,
	                                                  cubeMapFilename,
	                                                  nullptr,
	                                                  m_pTextureCubeSRV.GetAddressOf(),
	                                                  generateMips
	);
	
	if (FAILED(hr))
		return hr;

	return InitResource(device, skySphereRadius);
}

ID3D11ShaderResourceView* SkyRender::GetTextureCube() const
{
	return m_pTextureCubeSRV.Get();
}

void SkyRender::Draw(ID3D11DeviceContext* deviceContext, SkyEffect& skyEffect, const Camera& camera)
{
	UINT strides[1] = { sizeof(XMFLOAT3) };
	UINT offsets[1] = { 0 };
	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), strides, offsets);
	deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	skyEffect.SetWorldViewProjMatrix(XMMatrixTranslationFromVector(camera.GetPositionVector()) * camera.GetViewProjMatrix());
	skyEffect.SetTextureCube(m_pTextureCubeSRV.Get());
	skyEffect.Apply(deviceContext);
	deviceContext->DrawIndexed(m_indexCount, 0, 0);
}

void SkyRender::SetDebugObjectName(const std::string& name) const
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	// 先清空可能存在的名称
	D3D11SetDebugObjectName(m_pTextureCubeSRV.Get(), nullptr);

	D3D11SetDebugObjectName(m_pTextureCubeSRV.Get(), name + ".CubeMapSRV");
	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), name + ".VertexBuffer");
	D3D11SetDebugObjectName(m_pIndexBuffer.Get(), name + ".IndexBuffer");
#else
	UNREFERENCED_PARAMETER(name);
#endif
}

HRESULT SkyRender::InitResource(ID3D11Device* device, const float skySphereRadius)
{
	auto sphere = Geometry::CreateSphere<VertexPos>(skySphereRadius);

	// 顶点缓冲区创建
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(XMFLOAT3) * static_cast<UINT>(sphere.vertexVec.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = sphere.vertexVec.data();

	const HRESULT hr = device->CreateBuffer(&vbd, &InitData, &m_pVertexBuffer);
	
	if (FAILED(hr))
		return hr;

	// 索引缓冲区创建
	m_indexCount = static_cast<UINT>(sphere.indexVec.size());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(DWORD) * m_indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	InitData.pSysMem = sphere.indexVec.data();

	return device->CreateBuffer(&ibd, &InitData, &m_pIndexBuffer);
}
