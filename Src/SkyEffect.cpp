#include "SkyEffect.h"

#include <array>

using namespace DirectX;

//
// SkyEffect::Impl 需要先于SkyEffect的定义
//

class SkyEffect::Impl
{
public:
	std::unique_ptr<EffectHelper> m_pEffectHelper;
	std::shared_ptr<IEffectPass> m_pCurrEffectPass;

	ComPtr<ID3D11InputLayout> m_pVertexPosLayout;

	XMFLOAT4X4 m_world{};
	XMFLOAT4X4 m_view{};
	XMFLOAT4X4 m_proj{};
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

	m_pImpl->m_pEffectHelper = std::make_unique<EffectHelper>();

	ComPtr<ID3DBlob> blob;
	
	// ******************
	// 创建顶点着色器
	//

	HR(CreateShaderFromFile(L"HLSL\\Sky_VS.cso", L"HLSL\\Sky_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("Sky_VS", device, blob.Get()));
	// 创建顶点布局
	HR(device->CreateInputLayout(VertexPos::InputLayout, ARRAYSIZE(VertexPos::InputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pImpl->m_pVertexPosLayout.GetAddressOf()));

	// ******************
	// 创建像素着色器
	//
	HR(CreateShaderFromFile(L"HLSL\\Sky_PS.cso", L"HLSL\\Sky_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("Sky_PS", device, blob.Get()));


	// ******************
	// 创建通道
	//
	EffectPassDesc passDesc;
	passDesc.nameVS = "Sky_VS";
	passDesc.namePS = "Sky_PS";
	HR(m_pImpl->m_pEffectHelper->AddEffectPass("Sky", device, &passDesc));

	m_pImpl->m_pEffectHelper->SetSamplerStateByName("g_Sam", RenderStates::SSLinearWrap.Get());
	m_pImpl->m_pCurrEffectPass = m_pImpl->m_pEffectHelper->GetEffectPass("Sky");
	m_pImpl->m_pCurrEffectPass->SetDepthStencilState(RenderStates::DSSLessEqual.Get(), 0);
	m_pImpl->m_pCurrEffectPass->SetRasterizerState(RenderStates::RSNoCull.Get());

	// 设置调试对象名
	D3D11SetDebugObjectName(m_pImpl->m_pVertexPosLayout.Get(), "SkyEffect.VertexPosLayout");
	m_pImpl->m_pEffectHelper->SetDebugObjectName("SkyEffect");

	return true;
}

void SkyEffect::SetRenderDefault(ID3D11DeviceContext* deviceContext) const
{
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexPosLayout.Get());
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void XM_CALLCONV SkyEffect::SetWorldMatrix(FXMMATRIX world) const
{
	XMStoreFloat4x4(&m_pImpl->m_world, world);
}

void XM_CALLCONV SkyEffect::SetViewMatrix(FXMMATRIX view) const
{
	XMStoreFloat4x4(&m_pImpl->m_view, view);
}

void XM_CALLCONV SkyEffect::SetProjMatrix(FXMMATRIX proj) const
{
	XMStoreFloat4x4(&m_pImpl->m_proj, proj);
}

void SkyEffect::SetTextureCube(ID3D11ShaderResourceView* textureCube) const
{
	m_pImpl->m_pEffectHelper->SetShaderResourceByName("g_TexCube", textureCube);
}

void SkyEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	XMMATRIX worldViewProjMatrix = XMMatrixTranspose(XMLoadFloat4x4(&m_pImpl->m_world) * XMLoadFloat4x4(&m_pImpl->m_view) * XMLoadFloat4x4(&m_pImpl->m_proj));
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldViewProj")->SetFloatMatrix(4, 4, reinterpret_cast<const FLOAT*>(&worldViewProjMatrix));

	m_pImpl->m_pCurrEffectPass->Apply(deviceContext);
}
