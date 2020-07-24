#include "ScreenFadeEffect.h"

using namespace DirectX;

//
// ScreenFadeEffect::Impl 需要先于ScreenFadeEffect的定义
//

class ScreenFadeEffect::Impl : public AlignedType<Impl>
{
public:

	//
	// 这些结构体对应HLSL的结构体。需要按16字节对齐
	//

	struct CBChangesEveryFrame
	{
		float fadeAmount;
		XMFLOAT3 pad;
	};

	struct CBChangesRarely
	{
		XMMATRIX worldViewProj;
	};

	// 必须显式指定
	Impl() : m_isDirty(), m_pCBuffers() {}
	~Impl() = default;

	Impl(const Impl& other) = default;
	Impl(Impl&& other) noexcept = default;
	Impl& operator=(const Impl& other) = default;
	Impl& operator=(Impl&& other) noexcept = default;

	CBufferObject<0, CBChangesEveryFrame> m_cbFrame;		// 每帧修改的常量缓冲区
	CBufferObject<1, CBChangesRarely>	m_cbRarely;		    // 很少修改的常量缓冲区
	BOOL m_isDirty;										    // 是否有值变更
	std::array<CBufferBase*, 2> m_pCBuffers;				    // 统一管理上面所有的常量缓冲区

	ComPtr<ID3D11VertexShader> m_pScreenFadeVS;
	ComPtr<ID3D11PixelShader> m_pScreenFadePS;

	ComPtr<ID3D11InputLayout> m_pVertexPosTexLayout;

	ComPtr<ID3D11ShaderResourceView> m_pTexture;			// 用于淡入淡出的纹理
};



//
// ScreenFadeEffect
//

namespace
{
	// ScreenFadeEffect单例
	ScreenFadeEffect* g_pInstance = nullptr;
}

ScreenFadeEffect::ScreenFadeEffect()
{
	if (g_pInstance)
		throw std::exception("ScreenFadeEffect is a singleton!");
	g_pInstance = this;
	m_pImpl = std::make_unique<Impl>();
}

ScreenFadeEffect::~ScreenFadeEffect() = default;

ScreenFadeEffect::ScreenFadeEffect(ScreenFadeEffect&& moveFrom) noexcept
{
	m_pImpl.swap(moveFrom.m_pImpl);
}

ScreenFadeEffect& ScreenFadeEffect::operator=(ScreenFadeEffect&& moveFrom) noexcept
{
	m_pImpl.swap(moveFrom.m_pImpl);
	return *this;
}

ScreenFadeEffect& ScreenFadeEffect::Get()
{
	if (!g_pInstance)
		throw std::exception("ScreenFadeEffect needs an instance!");
	return *g_pInstance;
}

bool ScreenFadeEffect::InitAll(ID3D11Device* device) const
{
	if (!device)
		return false;

	if (!RenderStates::IsInit())
		throw std::exception("RenderStates need to be initialized first!");

	ComPtr<ID3DBlob> blob;

	// ******************
	// 创建顶点着色器
	//

	HR(CreateShaderFromFile(L"HLSL\\ScreenFade_VS.cso", L"HLSL\\ScreenFade_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pImpl->m_pScreenFadeVS.GetAddressOf()));
	// 创建顶点布局
	HR(device->CreateInputLayout(VertexPosTex::InputLayout, ARRAYSIZE(VertexPosTex::InputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pImpl->m_pVertexPosTexLayout.GetAddressOf()));

	// ******************
	// 创建像素着色器
	//

	HR(CreateShaderFromFile(L"HLSL\\ScreenFade_PS.cso", L"HLSL\\ScreenFade_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pImpl->m_pScreenFadePS.GetAddressOf()));



	m_pImpl->m_pCBuffers = {
		&m_pImpl->m_cbFrame,
		&m_pImpl->m_cbRarely
	};

	// 创建常量缓冲区
	for (auto& pBuffer : m_pImpl->m_pCBuffers)
	{
		HR(pBuffer->CreateBuffer(device));
	}

	// 设置调试对象名
	D3D11SetDebugObjectName(m_pImpl->m_pVertexPosTexLayout.Get(), "ScreenFadeEffect.VertexPosTexLayout");
	D3D11SetDebugObjectName(m_pImpl->m_pCBuffers[0]->cBuffer.Get(), "ScreenFadeEffect.CBFrame");
	D3D11SetDebugObjectName(m_pImpl->m_pCBuffers[1]->cBuffer.Get(), "ScreenFadeEffect.CBRarely");
	D3D11SetDebugObjectName(m_pImpl->m_pScreenFadeVS.Get(), "ScreenFadeEffect.ScreenFade_VS");
	D3D11SetDebugObjectName(m_pImpl->m_pScreenFadePS.Get(), "ScreenFadeEffect.ScreenFade_PS");

	return true;
}

void ScreenFadeEffect::SetRenderDefault(ID3D11DeviceContext* deviceContext) const
{
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexPosTexLayout.Get());
	deviceContext->VSSetShader(m_pImpl->m_pScreenFadeVS.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_pImpl->m_pScreenFadePS.Get(), nullptr, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->GSSetShader(nullptr, nullptr, 0);
	deviceContext->RSSetState(nullptr);

	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void XM_CALLCONV ScreenFadeEffect::SetWorldViewProjMatrix(FXMMATRIX world, CXMMATRIX view, CXMMATRIX proj) const
{
	auto& cBuffer = m_pImpl->m_cbRarely;
	cBuffer.data.worldViewProj = XMMatrixTranspose(world * view * proj);
	m_pImpl->m_isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV ScreenFadeEffect::SetWorldViewProjMatrix(FXMMATRIX worldViewProjMatrix) const
{
	auto& cBuffer = m_pImpl->m_cbRarely;
	cBuffer.data.worldViewProj = XMMatrixTranspose(worldViewProjMatrix);
	m_pImpl->m_isDirty = cBuffer.isDirty = true;
}

void ScreenFadeEffect::SetFadeAmount(const float fadeAmount) const
{
	auto& cBuffer = m_pImpl->m_cbFrame;
	cBuffer.data.fadeAmount = fadeAmount;
	m_pImpl->m_isDirty = cBuffer.isDirty = true;
}

void ScreenFadeEffect::SetTexture(ID3D11ShaderResourceView* texture) const
{
	m_pImpl->m_pTexture = texture;
}

void ScreenFadeEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	auto& pCBuffers = m_pImpl->m_pCBuffers;
	// 将缓冲区绑定到渲染管线上
	pCBuffers[0]->BindPS(deviceContext);
	pCBuffers[1]->BindVS(deviceContext);
	// 设置SRV
	deviceContext->PSSetShaderResources(0, 1, m_pImpl->m_pTexture.GetAddressOf());

	if (m_pImpl->m_isDirty)
	{
		m_pImpl->m_isDirty = false;
		for (auto& pCBuffer : pCBuffers)
		{
			pCBuffer->UpdateBuffer(deviceContext);
		}
	}
}
