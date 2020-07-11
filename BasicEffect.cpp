#include "Effects.h"
#include "d3dUtil.h"
#include "EffectHelper.h"	// 必须晚于Effects.h和d3dUtil.h包含
#include "DXTrace.h"
#include "Vertex.h"

using namespace DirectX;

//
// BasicEffect::Impl 需要先于BasicEffect的定义
//

class BasicEffect::Impl : public AlignedType<Impl>
{
public:

	//
	// 这些结构体对应HLSL的结构体。需要按16字节对齐
	//

	struct CBChangesEveryDrawing
	{
		XMMATRIX m_world;
		XMMATRIX m_worldInvTranspose;
		Material m_material;
	};

	struct CBDrawingStates
	{
		int m_isReflection;
		int m_isShadow;
		XMINT2 m_pad;
	};

	struct CBChangesEveryFrame
	{
		XMMATRIX m_view;
		XMVECTOR m_eyePos;
	};

	struct CBChangesOnResize
	{
		XMMATRIX m_proj;
	};


	struct CBChangesRarely
	{
		XMMATRIX m_reflection;
		XMMATRIX m_shadow;
		XMMATRIX m_refShadow;
		DirectionalLight m_dirLight[MaxLights];
		PointLight m_pointLight[MaxLights];
		SpotLight m_spotLight[MaxLights];
	};

	// 必须显式指定
	Impl() : m_isDirty() {}
	~Impl() = default;

	Impl(const Impl& other) = default;
	Impl(Impl&& other) noexcept = default;
	Impl& operator=(const Impl& other) = default;
	Impl& operator=(Impl&& other) noexcept = default;

	// 需要16字节对齐的优先放在前面
	CBufferObject<0, CBChangesEveryDrawing> m_cbDrawing;		// 每次对象绘制的常量缓冲区
	CBufferObject<1, CBDrawingStates>       m_cbStates;		    // 每次绘制状态变更的常量缓冲区
	CBufferObject<2, CBChangesEveryFrame>   m_cbFrame;		    // 每帧绘制的常量缓冲区
	CBufferObject<3, CBChangesOnResize>     m_cbOnResize;		// 每次窗口大小变更的常量缓冲区
	CBufferObject<4, CBChangesRarely>		m_cbRarely;		    // 几乎不会变更的常量缓冲区
	BOOL m_isDirty;												// 是否有值变更
	std::vector<CBufferBase*> m_pCBuffers;					    // 统一管理上面所有的常量缓冲区

	ComPtr<ID3D11VertexShader> m_pVertexShader3D;				// 用于3D的顶点着色器
	ComPtr<ID3D11PixelShader>  m_pPixelShader3D;				// 用于3D的像素着色器
	ComPtr<ID3D11VertexShader> m_pVertexShader2D;				// 用于2D的顶点着色器
	ComPtr<ID3D11PixelShader>  m_pPixelShader2D;				// 用于2D的像素着色器

	ComPtr<ID3D11InputLayout>  m_pVertexLayout2D;				// 用于2D的顶点输入布局
	ComPtr<ID3D11InputLayout>  m_pVertexLayout3D;				// 用于3D的顶点输入布局

	ComPtr<ID3D11ShaderResourceView> m_pTexture;				// 用于绘制的纹理
};

//
// BasicEffect
//

namespace
{
	// BasicEffect单例
	BasicEffect* g_pInstance = nullptr;
}

BasicEffect::BasicEffect()
{
	if (g_pInstance)
		throw std::exception("BasicEffect is a singleton!");
	g_pInstance = this;
	m_pImpl = std::make_unique<Impl>();
}

BasicEffect::~BasicEffect() = default;

BasicEffect::BasicEffect(BasicEffect&& moveFrom) noexcept
{
	m_pImpl.swap(moveFrom.m_pImpl);
}

BasicEffect& BasicEffect::operator=(BasicEffect&& moveFrom) noexcept
{
	m_pImpl.swap(moveFrom.m_pImpl);
	return *this;
}

BasicEffect& BasicEffect::Get()
{
	if (!g_pInstance)
		throw std::exception("BasicEffect needs an instance!");
	return *g_pInstance;
}


bool BasicEffect::InitAll(ID3D11Device* device) const
{
	if (!device)
		return false;

	if (!m_pImpl->m_pCBuffers.empty())
		return true;

	if (!RenderStates::IsInit())
		throw std::exception("RenderStates need to be initialized first!");

	ComPtr<ID3DBlob> blob;

	// 创建顶点着色器(2D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_VS_2D.cso", L"HLSL\\Basic_VS_2D.hlsl", "VS_2D", "vs_5_0", blob.GetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pImpl->m_pVertexShader2D.GetAddressOf()));
	// 创建顶点布局(2D)
	HR(device->CreateInputLayout(VertexPosTex::InputLayout, ARRAYSIZE(VertexPosTex::InputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pImpl->m_pVertexLayout2D.GetAddressOf()));

	// 创建像素着色器(2D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_PS_2D.cso", L"HLSL\\Basic_PS_2D.hlsl", "PS_2D", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pImpl->m_pPixelShader2D.GetAddressOf()));

	// 创建顶点着色器(3D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_VS_3D.cso", L"HLSL\\Basic_VS_3D.hlsl", "VS_3D", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pImpl->m_pVertexShader3D.GetAddressOf()));
	// 创建顶点布局(3D)
	HR(device->CreateInputLayout(VertexPosNormalTex::InputLayout, ARRAYSIZE(VertexPosNormalTex::InputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pImpl->m_pVertexLayout3D.GetAddressOf()));

	// 创建像素着色器(3D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_PS_3D.cso", L"HLSL\\Basic_PS_3D.hlsl", "PS_3D", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pImpl->m_pPixelShader3D.GetAddressOf()));


	m_pImpl->m_pCBuffers.assign({
		&m_pImpl->m_cbDrawing, 
		&m_pImpl->m_cbFrame, 
		&m_pImpl->m_cbStates, 
		&m_pImpl->m_cbOnResize, 
		&m_pImpl->m_cbRarely});

	// 创建常量缓冲区
	for (auto& pBuffer : m_pImpl->m_pCBuffers)
	{
		HR(pBuffer->CreateBuffer(device));
	}

	// 设置调试对象名
	D3D11SetDebugObjectName(m_pImpl->m_pVertexLayout2D.Get(), "VertexPosTexLayout");
	D3D11SetDebugObjectName(m_pImpl->m_pVertexLayout3D.Get(), "VertexPosNormalTexLayout");
	D3D11SetDebugObjectName(m_pImpl->m_pCBuffers[0]->m_cBuffer.Get(), "CBDrawing");
	D3D11SetDebugObjectName(m_pImpl->m_pCBuffers[1]->m_cBuffer.Get(), "CBStates");
	D3D11SetDebugObjectName(m_pImpl->m_pCBuffers[2]->m_cBuffer.Get(), "CBFrame");
	D3D11SetDebugObjectName(m_pImpl->m_pCBuffers[3]->m_cBuffer.Get(), "CBOnResize");
	D3D11SetDebugObjectName(m_pImpl->m_pCBuffers[4]->m_cBuffer.Get(), "CBRarely");
	D3D11SetDebugObjectName(m_pImpl->m_pVertexShader2D.Get(), "Basic_VS_2D");
	D3D11SetDebugObjectName(m_pImpl->m_pVertexShader3D.Get(), "Basic_VS_3D");
	D3D11SetDebugObjectName(m_pImpl->m_pPixelShader2D.Get(), "Basic_PS_2D");
	D3D11SetDebugObjectName(m_pImpl->m_pPixelShader3D.Get(), "Basic_PS_3D");

	return true;
}

void BasicEffect::SetRenderDefault(ID3D11DeviceContext* deviceContext) const
{
	/*
		图元类型									含义
		D3D11_PRIMITIVE_TOPOLOGY_POINTLIST			按一系列点进行装配
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP			按一系列线段进行装配，每相邻两个顶点(或索引数组相邻的两个索引对应的顶点)构成一条线段
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST			按一系列线段进行装配，每两个顶点(或索引数组每两个索引对应的顶点)构成一条线段
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP		按一系列三角形进行装配，每相邻三个顶点(或索引数组相邻的三个索引对应的顶点)构成一个三角形
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST		按一系列三角形进行装配，每三个顶点(或索引数组每三个索引对应的顶点)构成一个三角形
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ		每4个顶点为一组，只绘制第2个顶点与第3个顶点的连线（或索引数组每4个索引为一组，只绘制索引模4余数为2和3的连线）
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ		绘制除了最开始和结尾的所有线段(或者索引数组不绘制索引0和1的连线，以及n-2和n-1的连线)
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ	每6个顶点为一组，只绘制第1、3、5个顶点构成的三角形(或索引数组每6个索引为一组，只绘制索引模6余数为0, 2, 4的三角形)
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ	抛弃所有索引模2为奇数的顶点或索引，剩余的进行Triangle Strip的绘制
	 */
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexLayout3D.Get());
	deviceContext->VSSetShader(m_pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->PSSetShader(m_pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	/*
		void ID3D11DeviceContext::OMSetBlendState(
			ID3D11BlendState *pBlendState,      // [In]混合状态，如果要使用默认混合状态则提供nullptr
			const FLOAT [4]  BlendFactor,       // [In]混合因子，如不需要可以为nullptr
			UINT             SampleMask);       // [In]采样掩码，默认为0xffffffff
	 */
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetRenderAlphaBlend(ID3D11DeviceContext* deviceContext) const
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexLayout3D.Get());
	deviceContext->VSSetShader(m_pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(m_pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetDrawBoltAnimNoDepthTest(ID3D11DeviceContext* deviceContext) const
{
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexLayout3D.Get());
	deviceContext->VSSetShader(m_pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(m_pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDepthTest.Get(), 0);
	deviceContext->OMSetBlendState(RenderStates::BSAdditive.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetDrawBoltAnimNoDepthWrite(ID3D11DeviceContext* deviceContext) const
{
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexLayout3D.Get());
	deviceContext->VSSetShader(m_pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(m_pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDepthWrite.Get(), 0);
	deviceContext->OMSetBlendState(RenderStates::BSAdditive.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetRenderNoDoubleBlend(ID3D11DeviceContext* deviceContext, const UINT stencilRef) const
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexLayout3D.Get());
	deviceContext->VSSetShader(m_pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(m_pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDoubleBlend.Get(), stencilRef);
	deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetWriteStencilOnly(ID3D11DeviceContext* deviceContext, const UINT stencilRef) const
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexLayout3D.Get());
	deviceContext->VSSetShader(m_pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->PSSetShader(m_pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSWriteStencil.Get(), stencilRef);
	deviceContext->OMSetBlendState(RenderStates::BSNoColorWrite.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetRenderDefaultWithStencil(ID3D11DeviceContext* deviceContext, const UINT stencilRef) const
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexLayout3D.Get());
	deviceContext->VSSetShader(m_pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSCullClockWise.Get());
	deviceContext->PSSetShader(m_pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), stencilRef);
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetRenderAlphaBlendWithStencil(ID3D11DeviceContext* deviceContext, const UINT stencilRef) const
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexLayout3D.Get());
	deviceContext->VSSetShader(m_pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(m_pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), stencilRef);
	deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetDrawBoltAnimNoDepthTestWithStencil(ID3D11DeviceContext* deviceContext, const UINT stencilRef) const
{
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexLayout3D.Get());
	deviceContext->VSSetShader(m_pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(m_pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDepthTestWithStencil.Get(), stencilRef);
	deviceContext->OMSetBlendState(RenderStates::BSAdditive.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetDrawBoltAnimNoDepthWriteWithStencil(ID3D11DeviceContext* deviceContext, const UINT stencilRef) const
{
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexLayout3D.Get());
	deviceContext->VSSetShader(m_pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(m_pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDepthWriteWithStencil.Get(), stencilRef);
	deviceContext->OMSetBlendState(RenderStates::BSAdditive.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::Set2DRenderDefault(ID3D11DeviceContext* deviceContext) const
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexLayout2D.Get());
	deviceContext->VSSetShader(m_pImpl->m_pVertexShader2D.Get(), nullptr, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->PSSetShader(m_pImpl->m_pPixelShader2D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void BasicEffect::Set2DRenderAlphaBlend(ID3D11DeviceContext* deviceContext) const
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexLayout2D.Get());
	deviceContext->VSSetShader(m_pImpl->m_pVertexShader2D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(m_pImpl->m_pPixelShader2D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void XM_CALLCONV BasicEffect::SetWorldMatrix(const FXMMATRIX& world) const
{
	auto& cBuffer = m_pImpl->m_cbDrawing;
	cBuffer.m_data.m_world = XMMatrixTranspose(world);
	cBuffer.m_data.m_worldInvTranspose = XMMatrixInverse(nullptr, world);	// 两次转置抵消
	m_pImpl->m_isDirty = cBuffer.m_isDirty = true;
}

void XM_CALLCONV BasicEffect::SetViewMatrix(const FXMMATRIX& view) const
{
	auto& cBuffer = m_pImpl->m_cbFrame;
	cBuffer.m_data.m_view = XMMatrixTranspose(view);
	m_pImpl->m_isDirty = cBuffer.m_isDirty = true;
}

void XM_CALLCONV BasicEffect::SetProjMatrix(const FXMMATRIX& proj) const
{
	auto& cBuffer = m_pImpl->m_cbOnResize;
	cBuffer.m_data.m_proj = XMMatrixTranspose(proj);
	m_pImpl->m_isDirty = cBuffer.m_isDirty = true;
}

void XM_CALLCONV BasicEffect::SetReflectionMatrix(const FXMMATRIX& reflection) const
{
	auto& cBuffer = m_pImpl->m_cbRarely;
	cBuffer.m_data.m_reflection = XMMatrixTranspose(reflection);
	m_pImpl->m_isDirty = cBuffer.m_isDirty = true;
}

void XM_CALLCONV BasicEffect::SetShadowMatrix(const FXMMATRIX& shadow) const
{
	auto& cBuffer = m_pImpl->m_cbRarely;
	cBuffer.m_data.m_shadow = XMMatrixTranspose(shadow);
	m_pImpl->m_isDirty = cBuffer.m_isDirty = true;
}

void XM_CALLCONV BasicEffect::SetRefShadowMatrix(const FXMMATRIX& refShadow) const
{
	auto& cBuffer = m_pImpl->m_cbRarely;
	cBuffer.m_data.m_refShadow = XMMatrixTranspose(refShadow);
	m_pImpl->m_isDirty = cBuffer.m_isDirty = true;
}

void BasicEffect::SetDirLight(const size_t pos, const DirectionalLight& dirLight) const
{
	auto& cBuffer = m_pImpl->m_cbRarely;
	cBuffer.m_data.m_dirLight[pos] = dirLight;
	m_pImpl->m_isDirty = cBuffer.m_isDirty = true;
}

void BasicEffect::SetPointLight(const size_t pos, const PointLight& pointLight) const
{
	auto& cBuffer = m_pImpl->m_cbRarely;
	cBuffer.m_data.m_pointLight[pos] = pointLight;
	m_pImpl->m_isDirty = cBuffer.m_isDirty = true;
}

void BasicEffect::SetSpotLight(const size_t pos, const SpotLight& spotLight) const
{
	auto& cBuffer = m_pImpl->m_cbRarely;
	cBuffer.m_data.m_spotLight[pos] = spotLight;
	m_pImpl->m_isDirty = cBuffer.m_isDirty = true;
}

void BasicEffect::SetMaterial(const Material& material) const
{
	auto& cBuffer = m_pImpl->m_cbDrawing;
	cBuffer.m_data.m_material = material;
	m_pImpl->m_isDirty = cBuffer.m_isDirty = true;
}

void BasicEffect::SetTexture(ID3D11ShaderResourceView* texture) const
{
	m_pImpl->m_pTexture = texture;
}

void XM_CALLCONV BasicEffect::SetEyePos(const FXMVECTOR& eyePos) const
{
	auto& cBuffer = m_pImpl->m_cbFrame;
	cBuffer.m_data.m_eyePos = eyePos;
	m_pImpl->m_isDirty = cBuffer.m_isDirty = true;
}

void BasicEffect::SetReflectionState(const bool isOn) const
{
	auto& cBuffer = m_pImpl->m_cbStates;
	cBuffer.m_data.m_isReflection = isOn;
	m_pImpl->m_isDirty = cBuffer.m_isDirty = true;
}

void BasicEffect::SetShadowState(const bool isOn) const
{
	auto& cBuffer = m_pImpl->m_cbStates;
	cBuffer.m_data.m_isShadow = isOn;
	m_pImpl->m_isDirty = cBuffer.m_isDirty = true;
}

void BasicEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	auto& pCBuffers = m_pImpl->m_pCBuffers;
	// 将缓冲区绑定到渲染管线上
	pCBuffers[0]->BindVS(deviceContext);
	pCBuffers[1]->BindVS(deviceContext);
	pCBuffers[2]->BindVS(deviceContext);
	pCBuffers[3]->BindVS(deviceContext);
	pCBuffers[4]->BindVS(deviceContext);

	pCBuffers[0]->BindPS(deviceContext);
	pCBuffers[1]->BindPS(deviceContext);
	pCBuffers[2]->BindPS(deviceContext);
	pCBuffers[4]->BindPS(deviceContext);

	// 设置纹理
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
