#include "TextureRender.h"

#pragma warning(disable: 26812)

using namespace DirectX;

HRESULT TextureRender::InitResource(ID3D11Device* device, int texWidth, int texHeight, bool shadowMap, bool generateMips)
{
	// 防止重复初始化造成内存泄漏
	m_pOutputTextureSRV.Reset();
	m_pOutputTextureRTV.Reset();
	m_pOutputTextureDSV.Reset();
	m_pCacheRTV.Reset();
	m_pCacheDSV.Reset();

	m_shadowMap = shadowMap;
	m_generateMips = false;
	HRESULT hr;
	if (!m_shadowMap)
	{
		m_generateMips = generateMips;

		// ******************
		// 创建纹理
		//

		ComPtr<ID3D11Texture2D> texture;
		CD3D11_TEXTURE2D_DESC texDesc(DXGI_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, 1,
			(m_generateMips ? 0 : 1), D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
			D3D11_USAGE_DEFAULT, 0, 1, 0, (m_generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0));

		hr = device->CreateTexture2D(&texDesc, nullptr, texture.GetAddressOf());
		if (FAILED(hr))
			return hr;

		// ******************
		// 创建纹理对应的渲染目标视图
		//

		CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(texture.Get(), D3D11_RTV_DIMENSION_TEXTURE2D);

		hr = device->CreateRenderTargetView(texture.Get(), &rtvDesc, m_pOutputTextureRTV.GetAddressOf());
		if (FAILED(hr))
			return hr;

		// ******************
		// 创建纹理对应的着色器资源视图
		//

		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(texture.Get(), D3D11_SRV_DIMENSION_TEXTURE2D);

		hr = device->CreateShaderResourceView(texture.Get(), &srvDesc,
			m_pOutputTextureSRV.GetAddressOf());
		if (FAILED(hr))
			return hr;
	}

	// ******************
	// 创建与纹理等宽高的深度/模板缓冲区或阴影贴图，以及对应的视图
	//
	CD3D11_TEXTURE2D_DESC texDesc((m_shadowMap ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_D24_UNORM_S8_UINT),
		texWidth, texHeight, 1, 1,
		D3D11_BIND_DEPTH_STENCIL | (m_shadowMap ? D3D11_BIND_SHADER_RESOURCE : 0));

	ComPtr<ID3D11Texture2D> depthTex;
	hr = device->CreateTexture2D(&texDesc, nullptr, depthTex.GetAddressOf());
	if (FAILED(hr))
		return hr;

	CD3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc(depthTex.Get(), D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT);

	hr = device->CreateDepthStencilView(depthTex.Get(), &dsvDesc,
		m_pOutputTextureDSV.GetAddressOf());
	if (FAILED(hr))
		return hr;

	if (m_shadowMap)
	{
		// 阴影贴图的SRV
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(depthTex.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

		hr = device->CreateShaderResourceView(depthTex.Get(), &srvDesc,
			m_pOutputTextureSRV.GetAddressOf());
		if (FAILED(hr))
			return hr;
	}

	// ******************
	// 初始化视口
	//
	m_outputViewPort.TopLeftX = 0.0f;
	m_outputViewPort.TopLeftY = 0.0f;
	m_outputViewPort.Width = static_cast<float>(texWidth);
	m_outputViewPort.Height = static_cast<float>(texHeight);
	m_outputViewPort.MinDepth = 0.0f;
	m_outputViewPort.MaxDepth = 1.0f;

	return S_OK;
}

void TextureRender::Begin(ID3D11DeviceContext* deviceContext, const FLOAT backgroundColor[4])
{
	// 缓存渲染目标和深度模板视图
	deviceContext->OMGetRenderTargets(1, m_pCacheRTV.GetAddressOf(), m_pCacheDSV.GetAddressOf());
	// 缓存视口
	UINT numViewports = 1;
	deviceContext->RSGetViewports(&numViewports, &m_cacheViewPort);


	// 清空缓冲区
	if (!m_shadowMap)
	{
		deviceContext->ClearRenderTargetView(m_pOutputTextureRTV.Get(), backgroundColor);

	}
	deviceContext->ClearDepthStencilView(m_pOutputTextureDSV.Get(), D3D11_CLEAR_DEPTH | (m_shadowMap ? 0 : D3D11_CLEAR_STENCIL), 1.0f, 0);
	// 设置渲染目标和深度模板视图
	deviceContext->OMSetRenderTargets((m_shadowMap ? 0 : 1),
		(m_shadowMap ? nullptr : m_pOutputTextureRTV.GetAddressOf()),
		m_pOutputTextureDSV.Get());
	// 设置视口
	deviceContext->RSSetViewports(1, &m_outputViewPort);
}

void TextureRender::End(ID3D11DeviceContext* deviceContext)
{
	// 恢复默认设定
	deviceContext->RSSetViewports(1, &m_cacheViewPort);
	deviceContext->OMSetRenderTargets(1, m_pCacheRTV.GetAddressOf(), m_pCacheDSV.Get());

	// 若之前有指定需要mipmap链，则生成
	if (m_generateMips)
	{
		deviceContext->GenerateMips(m_pOutputTextureSRV.Get());
	}

	// 清空临时缓存的渲染目标视图和深度模板视图
	m_pCacheDSV.Reset();
	m_pCacheRTV.Reset();
}

ID3D11ShaderResourceView* TextureRender::GetOutputTexture() const
{
	return m_pOutputTextureSRV.Get();
}

void TextureRender::SetDebugObjectName(const std::string& name) const
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	D3D11SetDebugObjectName(m_pOutputTextureDSV.Get(), name + ".TextureDSV");
	D3D11SetDebugObjectName(m_pOutputTextureSRV.Get(), name + ".TextureSRV");
	if(!m_shadowMap)
	{
		// 作为阴影RTV是空的
		D3D11SetDebugObjectName(m_pOutputTextureRTV.Get(), name + ".TextureRTV");
	}
#else
	UNREFERENCED_PARAMETER(name);
#endif
}
