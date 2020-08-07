#include "ShadowEffect.h"

using namespace DirectX;

//
// ShadowEffect::Impl 需要先于ShadowEffect的定义
//

class ShadowEffect::Impl
{
public:
	std::unique_ptr<EffectHelper> m_pEffectHelper;

	std::shared_ptr<IEffectPass> m_pCurrEffectPass;

	ComPtr<ID3D11InputLayout> m_pInstancePosNormalTexLayout;
	ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexLayout;

	XMFLOAT4X4 m_world{};
	XMFLOAT4X4 m_view{};
	XMFLOAT4X4 m_proj{};
	RenderType m_renderType = RenderType::RenderObject;
};

//
// ShadowEffect
//

namespace
{
	// ShadowEffect单例
	ShadowEffect* g_pInstance = nullptr;
}

ShadowEffect::ShadowEffect()
{
	if (g_pInstance)
		throw std::exception("ShadowEffect is a singleton!");
	g_pInstance = this;
	m_pImpl = std::make_unique<Impl>();
}

ShadowEffect::~ShadowEffect() = default;

ShadowEffect::ShadowEffect(ShadowEffect&& moveFrom) noexcept
{
	m_pImpl.swap(moveFrom.m_pImpl);
}

ShadowEffect& ShadowEffect::operator=(ShadowEffect&& moveFrom) noexcept
{
	m_pImpl.swap(moveFrom.m_pImpl);
	return *this;
}

ShadowEffect& ShadowEffect::Get()
{
	if (!g_pInstance)
		throw std::exception("ShadowEffect needs an instance!");
	return *g_pInstance;
}

bool ShadowEffect::InitAll(ID3D11Device* device) const
{
	if (!device)
		return false;

	if (!RenderStates::IsInit())
		throw std::exception("RenderStates need to be initialized first!");

	m_pImpl->m_pEffectHelper = std::make_unique<EffectHelper>();

	ComPtr<ID3DBlob> blob;

	// 实例输入布局
	D3D11_INPUT_ELEMENT_DESC shadowInstLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "World", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "World", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "World", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "World", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	// ******************
	// 创建顶点着色器
	//

	HR(CreateShaderFromFile(L"HLSL\\ShadowInstance_VS.cso", L"HLSL\\ShadowInstance_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("ShadowInstance_VS", device, blob.Get()));
	// 创建顶点布局
	HR(device->CreateInputLayout(shadowInstLayout, ARRAYSIZE(shadowInstLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pImpl->m_pInstancePosNormalTexLayout.GetAddressOf()));

	HR(CreateShaderFromFile(L"HLSL\\ShadowObject_VS.cso", L"HLSL\\ShadowObject_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("ShadowObject_VS", device, blob.Get()));
	// 创建顶点布局
	HR(device->CreateInputLayout(VertexPosNormalTex::InputLayout, ARRAYSIZE(VertexPosNormalTex::InputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pImpl->m_pVertexPosNormalTexLayout.GetAddressOf()));

	// ******************
	// 创建像素着色器
	//
	HR(CreateShaderFromFile(L"HLSL\\Shadow_PS.cso", L"HLSL\\Shadow_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("Shadow_PS", device, blob.Get()));

	// ******************
	// 创建通道
	//
	EffectPassDesc passDesc;
	passDesc.nameVS = "ShadowInstance_VS";
	HR(m_pImpl->m_pEffectHelper->AddEffectPass("ShadowInstance", device, &passDesc));
	m_pImpl->m_pEffectHelper->GetEffectPass("ShadowInstance")->SetRasterizerState(RenderStates::RSDepth.Get());

	passDesc.nameVS = "ShadowObject_VS";
	HR(m_pImpl->m_pEffectHelper->AddEffectPass("ShadowObject", device, &passDesc));
	m_pImpl->m_pEffectHelper->GetEffectPass("ShadowObject")->SetRasterizerState(RenderStates::RSDepth.Get());

	passDesc.nameVS = "ShadowInstance_VS";
	passDesc.namePS = "Shadow_PS";
	HR(m_pImpl->m_pEffectHelper->AddEffectPass("ShadowInstanceAlphaClip", device, &passDesc));
	m_pImpl->m_pEffectHelper->GetEffectPass("ShadowInstanceAlphaClip")->SetRasterizerState(RenderStates::RSDepth.Get());

	passDesc.nameVS = "ShadowObject_VS";
	passDesc.namePS = "Shadow_PS";
	HR(m_pImpl->m_pEffectHelper->AddEffectPass("ShadowObjectAlphaClip", device, &passDesc));
	m_pImpl->m_pEffectHelper->GetEffectPass("ShadowObjectAlphaClip")->SetRasterizerState(RenderStates::RSDepth.Get());

	m_pImpl->m_pEffectHelper->SetSamplerStateByName("g_Sam", RenderStates::SSLinearWrap.Get());

	// 设置调试对象名
	D3D11SetDebugObjectName(m_pImpl->m_pInstancePosNormalTexLayout.Get(), "ShadowEffect.InstancePosNormalTexLayout");
	D3D11SetDebugObjectName(m_pImpl->m_pVertexPosNormalTexLayout.Get(), "ShadowEffect.VertexPosNormalTexLayout");
	m_pImpl->m_pEffectHelper->SetDebugObjectName("ShadowEffect");

	return true;
}

void ShadowEffect::SetRenderDefault(ID3D11DeviceContext* deviceContext, const RenderType type) const
{
	if (type == RenderType::RenderInstance)
	{
		deviceContext->IASetInputLayout(m_pImpl->m_pInstancePosNormalTexLayout.Get());
		m_pImpl->m_pCurrEffectPass = m_pImpl->m_pEffectHelper->GetEffectPass("ShadowInstance");
	}
	else
	{
		deviceContext->IASetInputLayout(m_pImpl->m_pVertexPosNormalTexLayout.Get());
		m_pImpl->m_pCurrEffectPass = m_pImpl->m_pEffectHelper->GetEffectPass("ShadowObject");
	}
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pImpl->m_renderType = type;
}

void ShadowEffect::SetRenderAlphaClip(ID3D11DeviceContext* deviceContext, const RenderType type) const
{
	if (type == RenderType::RenderInstance)
	{
		deviceContext->IASetInputLayout(m_pImpl->m_pInstancePosNormalTexLayout.Get());
		m_pImpl->m_pCurrEffectPass = m_pImpl->m_pEffectHelper->GetEffectPass("ShadowInstanceAlphaClip");
	}
	else
	{
		deviceContext->IASetInputLayout(m_pImpl->m_pVertexPosNormalTexLayout.Get());
		m_pImpl->m_pCurrEffectPass = m_pImpl->m_pEffectHelper->GetEffectPass("ShadowObjectAlphaClip");
	}
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pImpl->m_renderType = type;
}

void ShadowEffect::SetTextureDiffuse(ID3D11ShaderResourceView* textureDiffuse) const
{
	m_pImpl->m_pEffectHelper->SetShaderResourceByName("g_DiffuseMap", textureDiffuse);
}

void XM_CALLCONV ShadowEffect::SetWorldMatrix(FXMMATRIX world) const
{
	XMStoreFloat4x4(&m_pImpl->m_world, world);
}

void XM_CALLCONV ShadowEffect::SetViewMatrix(FXMMATRIX view) const
{
	XMStoreFloat4x4(&m_pImpl->m_view, view);
}

void XM_CALLCONV ShadowEffect::SetProjMatrix(FXMMATRIX proj) const
{
	XMStoreFloat4x4(&m_pImpl->m_proj, proj);
}

void ShadowEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	if (m_pImpl->m_renderType == RenderType::RenderInstance)
	{
		XMMATRIX viewProjMatrix = XMMatrixTranspose(XMLoadFloat4x4(&m_pImpl->m_view) * XMLoadFloat4x4(&m_pImpl->m_proj));
		m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ViewProj")->SetFloatMatrix(4, 4, reinterpret_cast<const FLOAT*>(&viewProjMatrix));
	}
	else
	{
		XMMATRIX worldViewProjMatrix = XMMatrixTranspose(XMLoadFloat4x4(&m_pImpl->m_world) * XMLoadFloat4x4(&m_pImpl->m_view) * XMLoadFloat4x4(&m_pImpl->m_proj));
		m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldViewProj")->SetFloatMatrix(4, 4, reinterpret_cast<const FLOAT*>(&worldViewProjMatrix));
	}

	m_pImpl->m_pCurrEffectPass->Apply(deviceContext);
}

