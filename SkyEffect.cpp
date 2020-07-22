#include "SkyEffect.h"

#include <array>

using namespace DirectX;

//
// SkyEffect::Impl 需要先于SkyEffect的定义
//

class SkyEffect::Impl : public AlignedType<Impl>
{
public:
	//
	// 这些结构体对应HLSL的结构体。需要按16字节对齐
	//

	struct CBChangesEveryFrame
	{
		XMMATRIX worldViewProj;
	};

	// 必须显式指定
	Impl() : m_isDirty(), m_pCBuffers() {}
	~Impl() = default;

	Impl(const Impl& other) = default;
	Impl(Impl && other) noexcept = default;
	Impl& operator=(const Impl & other) = default;
	Impl& operator=(Impl && other) noexcept = default;

	CBufferObject<0, CBChangesEveryFrame> m_cbFrame;	        // 每帧绘制的常量缓冲区

	BOOL m_isDirty;										        // 是否有值变更
	std::array<CBufferBase*, 1> m_pCBuffers;				        // 统一管理上面所有的常量缓冲区

	ComPtr<ID3D11VertexShader> m_pSkyVS;
	ComPtr<ID3D11PixelShader> m_pSkyPS;

	ComPtr<ID3D11InputLayout> m_pVertexPosLayout;

	ComPtr<ID3D11ShaderResourceView> m_pTextureCube;			// 天空盒纹理
};

//
// SkyEffect
//

namespace
{
	// SkyEffect单例
	SkyEffect* g_pInstance = nullptr;
}

SkyEffect::SkyEffect()
{
	if (g_pInstance)
		throw std::exception("SkyEffect is a singleton!");
	g_pInstance = this;
	m_pImpl = std::make_unique<Impl>();
}

SkyEffect::~SkyEffect() = default;

SkyEffect::SkyEffect(SkyEffect && moveFrom) noexcept
{
	m_pImpl.swap(moveFrom.m_pImpl);
}

SkyEffect & SkyEffect::operator=(SkyEffect&& moveFrom) noexcept
{
	m_pImpl.swap(moveFrom.m_pImpl);
	return *this;
}

SkyEffect & SkyEffect::Get()
{
	if (!g_pInstance)
		throw std::exception("SkyEffect needs an instance!");
	return *g_pInstance;
}

bool SkyEffect::InitAll(ID3D11Device* device) const
{
	if (!device)
		return false;

	if (!RenderStates::IsInit())
		throw std::exception("RenderStates need to be initialized first!");

	ComPtr<ID3DBlob> blob;
	
	// ******************
	// 创建顶点着色器
	//

	HR(CreateShaderFromFile(L"HLSL\\Sky_VS.cso", L"HLSL\\Sky_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pImpl->m_pSkyVS.GetAddressOf()));
	// 创建顶点布局
	HR(device->CreateInputLayout(VertexPos::InputLayout, ARRAYSIZE(VertexPos::InputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pImpl->m_pVertexPosLayout.GetAddressOf()));

	// ******************
	// 创建像素着色器
	//

	HR(CreateShaderFromFile(L"HLSL\\Sky_PS.cso", L"HLSL\\Sky_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pImpl->m_pSkyPS.GetAddressOf()));


	m_pImpl->m_pCBuffers = {
		&m_pImpl->m_cbFrame,
	};

	// 创建常量缓冲区
	for (auto& pBuffer : m_pImpl->m_pCBuffers)
	{
		HR(pBuffer->CreateBuffer(device));
	}

	// 设置调试对象名
	D3D11SetDebugObjectName(m_pImpl->m_pVertexPosLayout.Get(), "SkyEffect.VertexPosLayout");
	D3D11SetDebugObjectName(m_pImpl->m_pCBuffers[0]->cBuffer.Get(), "SkyEffect.CBFrame");
	D3D11SetDebugObjectName(m_pImpl->m_pSkyVS.Get(), "SkyEffect.Sky_VS");
	D3D11SetDebugObjectName(m_pImpl->m_pSkyPS.Get(), "SkyEffect.Sky_PS");

	return true;
}

void SkyEffect::SetRenderDefault(ID3D11DeviceContext* deviceContext) const
{
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexPosLayout.Get());
	deviceContext->VSSetShader(m_pImpl->m_pSkyVS.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_pImpl->m_pSkyPS.Get(), nullptr, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->GSSetShader(nullptr, nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());

	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSLessEqual.Get(), 0);
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void XM_CALLCONV SkyEffect::SetWorldViewProjMatrix(FXMMATRIX world, CXMMATRIX view, CXMMATRIX proj) const
{
	auto& cBuffer = m_pImpl->m_cbFrame;
	cBuffer.data.worldViewProj = XMMatrixTranspose(world * view * proj);
	m_pImpl->m_isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV SkyEffect::SetWorldViewProjMatrix(FXMMATRIX worldViewProjMatrix) const
{
	auto& cBuffer = m_pImpl->m_cbFrame;
	cBuffer.data.worldViewProj = XMMatrixTranspose(worldViewProjMatrix);
	m_pImpl->m_isDirty = cBuffer.isDirty = true;
}

void SkyEffect::SetTextureCube(ID3D11ShaderResourceView* textureCube) const
{
	m_pImpl->m_pTextureCube = textureCube;
}

void SkyEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	auto& pCBuffers = m_pImpl->m_pCBuffers;
	// 将缓冲区绑定到渲染管线上
	pCBuffers[0]->BindVS(deviceContext);
	
	// 设置SRV
	deviceContext->PSSetShaderResources(0, 1, m_pImpl->m_pTextureCube.GetAddressOf());

	if (m_pImpl->m_isDirty)
	{
		m_pImpl->m_isDirty = false;
		for (auto& pCBuffer : pCBuffers)
		{
			pCBuffer->UpdateBuffer(deviceContext);
		}
	}
}

