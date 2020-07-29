#include "DebugEffect.h"

using namespace DirectX;

//
// DebugEffect::Impl 需要先于ShadowEffect的定义
//

class DebugEffect::Impl
{
public:
	std::unique_ptr<EffectHelper> m_pEffectHelper;

	std::shared_ptr<IEffectPass> m_pCurrEffectPass;

	ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexLayout;

	XMFLOAT4X4 m_world;
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;
};

//
// DebugEffect
//

namespace
{
	// DebugEffect单例
	DebugEffect* g_pInstance = nullptr;
}

DebugEffect::DebugEffect()
{
	if (g_pInstance)
		throw std::exception("DebugEffect is a singleton!");
	g_pInstance = this;
	m_pImpl = std::make_unique<Impl>();
}

DebugEffect::~DebugEffect() = default;

DebugEffect::DebugEffect(DebugEffect&& moveFrom) noexcept
{
	m_pImpl.swap(moveFrom.m_pImpl);
}

DebugEffect& DebugEffect::operator=(DebugEffect&& moveFrom) noexcept
{
	m_pImpl.swap(moveFrom.m_pImpl);
	return *this;
}

DebugEffect& DebugEffect::Get()
{
	if (!g_pInstance)
		throw std::exception("DebugEffect needs an instance!");
	return *g_pInstance;
}

bool DebugEffect::InitAll(ID3D11Device* device) const
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

	HR(CreateShaderFromFile(L"HLSL\\DebugTexture_VS.cso", L"HLSL\\DebugTexture_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("DebugTexture_VS", device, blob.Get()));
	// 创建顶点布局
	HR(device->CreateInputLayout(VertexPosNormalTex::InputLayout, ARRAYSIZE(VertexPosNormalTex::InputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pImpl->m_pVertexPosNormalTexLayout.GetAddressOf()));

	// ******************
	// 创建像素着色器
	//

	HR(CreateShaderFromFile(L"HLSL\\DebugTextureRGBA_PS.cso", L"HLSL\\DebugTextureRGBA_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("DebugTextureRGBA_PS", device, blob.Get()));

	HR(CreateShaderFromFile(L"HLSL\\DebugTextureOneComp_PS.cso", L"HLSL\\DebugTextureOneComp_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("DebugTextureOneComp_PS", device, blob.Get()));

	HR(CreateShaderFromFile(L"HLSL\\DebugTextureOneCompGray_PS.cso", L"HLSL\\DebugTextureOneCompGray_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("DebugTextureOneCompGray_PS", device, blob.Get()));

	// ******************
	// 创建通道
	//
	EffectPassDesc passDesc;
	passDesc.nameVS = "DebugTexture_VS";
	passDesc.namePS = "DebugTextureRGBA_PS";
	HR(m_pImpl->m_pEffectHelper->AddEffectPass("DebugTextureRGBA", device, &passDesc));
	passDesc.nameVS = "DebugTexture_VS";
	passDesc.namePS = "DebugTextureOneComp_PS";
	HR(m_pImpl->m_pEffectHelper->AddEffectPass("DebugTextureOneComp", device, &passDesc));
	passDesc.nameVS = "DebugTexture_VS";
	passDesc.namePS = "DebugTextureOneCompGray_PS";
	HR(m_pImpl->m_pEffectHelper->AddEffectPass("DebugTextureOneCompGray", device, &passDesc));

	// 设置采样器
	m_pImpl->m_pEffectHelper->SetSamplerStateByName("g_Sam", RenderStates::SSLinearWrap.Get());

	// 设置调试对象名
	D3D11SetDebugObjectName(m_pImpl->m_pVertexPosNormalTexLayout.Get(), "DebugEffect.VertexPosNormalTexLayout");
	m_pImpl->m_pEffectHelper->SetDebugObjectName("DebugEffect");

	return true;
}

void DebugEffect::SetRenderDefault(ID3D11DeviceContext* deviceContext) const
{
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexPosNormalTexLayout.Get());
	m_pImpl->m_pCurrEffectPass = m_pImpl->m_pEffectHelper->GetEffectPass("DebugTextureRGBA");
}

void DebugEffect::SetRenderOneComponent(ID3D11DeviceContext* deviceContext, int index) const
{
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexPosNormalTexLayout.Get());
	m_pImpl->m_pCurrEffectPass = m_pImpl->m_pEffectHelper->GetEffectPass("DebugTextureOneComp");
	m_pImpl->m_pCurrEffectPass->PSGetParamByName("index")->SetSInt(index);
}

void DebugEffect::SetRenderOneComponentGray(ID3D11DeviceContext* deviceContext, int index) const
{
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexPosNormalTexLayout.Get());
	m_pImpl->m_pCurrEffectPass = m_pImpl->m_pEffectHelper->GetEffectPass("DebugTextureOneCompGray");
	m_pImpl->m_pCurrEffectPass->PSGetParamByName("index")->SetSInt(index);
}

void XM_CALLCONV DebugEffect::SetWorldMatrix(FXMMATRIX world) const
{
	XMStoreFloat4x4(&m_pImpl->m_world, world);
}

void XM_CALLCONV DebugEffect::SetViewMatrix(FXMMATRIX view) const
{
	XMStoreFloat4x4(&m_pImpl->m_view, view);
}

void XM_CALLCONV DebugEffect::SetProjMatrix(FXMMATRIX proj) const
{
	XMStoreFloat4x4(&m_pImpl->m_proj, proj);
}

void DebugEffect::SetTextureDiffuse(ID3D11ShaderResourceView* textureDiffuse) const
{
	m_pImpl->m_pEffectHelper->SetShaderResourceByName("g_DiffuseMap", textureDiffuse);
}

void DebugEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	XMMATRIX worldViewProjMatrix = XMMatrixTranspose(XMLoadFloat4x4(&m_pImpl->m_world) * XMLoadFloat4x4(&m_pImpl->m_view) * XMLoadFloat4x4(&m_pImpl->m_proj));
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldViewProj")->SetFloatMatrix(4, 4, reinterpret_cast<const FLOAT*>(&worldViewProjMatrix));

	m_pImpl->m_pCurrEffectPass->Apply(deviceContext);
}
