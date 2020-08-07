#include "MinimapEffect.h"

using namespace DirectX;

//
// MinimapEffect::Impl 需要先于MinimapEffect的定义
//

class MinimapEffect::Impl : public AlignedType<Impl>
{
public:

	//
	// 这些结构体对应HLSL的结构体。需要按16字节对齐
	//

	struct CBChangesEveryFrame
	{
		XMVECTOR eyePos;
	};

	struct CBDrawingStates
	{
		int fogEnabled;
		float visibleRange;
		XMFLOAT2 pad;
		XMVECTOR rect;
		XMVECTOR invisibleColor;
	};

	// 必须显式指定
	Impl() : m_isDirty(), m_pCBuffers() {}
	~Impl() = default;

	Impl(const Impl& other) = default;
	Impl(Impl&& other) noexcept = default;
	Impl& operator=(const Impl& other) = default;
	Impl& operator=(Impl&& other) noexcept = default;
	
	CBufferObject<0, CBChangesEveryFrame> m_cbFrame;		// 每帧修改的常量缓冲区
	CBufferObject<1, CBDrawingStates>	m_cbStates;		    // 每次绘制状态改变的常量缓冲区

	BOOL m_isDirty;										    // 是否有值变更
	std::array<CBufferBase*, 2> m_pCBuffers;				    // 统一管理上面所有的常量缓冲区

	ComPtr<ID3D11VertexShader> m_pMinimapVS;
	ComPtr<ID3D11PixelShader> m_pMinimapPS;

	ComPtr<ID3D11InputLayout> m_pVertexPosTexLayout;

	ComPtr<ID3D11ShaderResourceView> m_pTexture;			// 用于淡入淡出的纹理
};

//
// MinimapEffect
//

namespace
{
	// MinimapEffect单例
	MinimapEffect* g_pInstance = nullptr;
}

MinimapEffect::MinimapEffect()
{
	if (g_pInstance)
		throw std::exception("MinimapEffect is a singleton!");
	g_pInstance = this;
	m_pImpl = std::make_unique<Impl>();
}

MinimapEffect::~MinimapEffect() = default;

MinimapEffect::MinimapEffect(MinimapEffect&& moveFrom) noexcept
{
	m_pImpl.swap(moveFrom.m_pImpl);
}

MinimapEffect& MinimapEffect::operator=(MinimapEffect&& moveFrom) noexcept
{
	m_pImpl.swap(moveFrom.m_pImpl);
	return *this;
}

MinimapEffect & MinimapEffect::Get()
{
	if (!g_pInstance)
		throw std::exception("MinimapEffect needs an instance!");
	return *g_pInstance;
}

bool MinimapEffect::InitAll(ID3D11Device * device) const
{
	if (!device)
		return false;

	if (!RenderStates::IsInit())
		throw std::exception("RenderStates need to be initialized first!");

	ComPtr<ID3DBlob> blob;

	// ******************
	// 创建顶点着色器
	//

	HR(CreateShaderFromFile(L"HLSL\\Minimap_VS.cso", L"HLSL\\Minimap_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pImpl->m_pMinimapVS.GetAddressOf()));
	// 创建顶点布局
	HR(device->CreateInputLayout(VertexPosTex::InputLayout, ARRAYSIZE(VertexPosTex::InputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pImpl->m_pVertexPosTexLayout.GetAddressOf()));

	// ******************
	// 创建像素着色器
	//

	HR(CreateShaderFromFile(L"HLSL\\Minimap_PS.cso", L"HLSL\\Minimap_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pImpl->m_pMinimapPS.GetAddressOf()));


	m_pImpl->m_pCBuffers = {
		&m_pImpl->m_cbFrame,
		&m_pImpl->m_cbStates
	};

	// 创建常量缓冲区
	for (auto& pBuffer : m_pImpl->m_pCBuffers)
	{
		HR(pBuffer->CreateBuffer(device));
	}

	// 设置调试对象名
	D3D11SetDebugObjectName(m_pImpl->m_pVertexPosTexLayout.Get(), "MinimapEffect.VertexPosTexLayout");
	D3D11SetDebugObjectName(m_pImpl->m_pCBuffers[0]->cBuffer.Get(), "MinimapEffect.CBFrame");
	D3D11SetDebugObjectName(m_pImpl->m_pCBuffers[1]->cBuffer.Get(), "MinimapEffect.CBStates");
	D3D11SetDebugObjectName(m_pImpl->m_pMinimapVS.Get(), "MinimapEffect.Minimap_VS");
	D3D11SetDebugObjectName(m_pImpl->m_pMinimapPS.Get(), "MinimapEffect.Minimap_PS");

	return true;
}

void MinimapEffect::SetRenderDefault(ID3D11DeviceContext* deviceContext) const
{
	deviceContext->IASetInputLayout(m_pImpl->m_pVertexPosTexLayout.Get());
	deviceContext->VSSetShader(m_pImpl->m_pMinimapVS.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_pImpl->m_pMinimapPS.Get(), nullptr, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->GSSetShader(nullptr, nullptr, 0);
	deviceContext->RSSetState(nullptr);

	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDepthTest.Get(), 0);	// 关闭深度测试
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void MinimapEffect::SetFogState(const bool isOn) const
{
	auto& cBuffer = m_pImpl->m_cbStates;
	cBuffer.data.fogEnabled = isOn;
	m_pImpl->m_isDirty = cBuffer.isDirty = true;
}

void MinimapEffect::SetVisibleRange(const float range) const
{
	auto& cBuffer = m_pImpl->m_cbStates;
	cBuffer.data.visibleRange = range;
	m_pImpl->m_isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV MinimapEffect::SetEyePos(FXMVECTOR eyePos) const
{
	auto& cBuffer = m_pImpl->m_cbFrame;
	cBuffer.data.eyePos = eyePos;
	m_pImpl->m_isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV MinimapEffect::SetMinimapRect(FXMVECTOR rect) const
{
	auto& cBuffer = m_pImpl->m_cbStates;
	cBuffer.data.rect = rect;
	m_pImpl->m_isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV MinimapEffect::SetInvisibleColor(FXMVECTOR color) const
{
	auto& cBuffer = m_pImpl->m_cbStates;
	cBuffer.data.invisibleColor = color;
	m_pImpl->m_isDirty = cBuffer.isDirty = true;
}

void MinimapEffect::SetTexture(ID3D11ShaderResourceView* texture) const
{
	m_pImpl->m_pTexture = texture;
}

void MinimapEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	auto& pCBuffers = m_pImpl->m_pCBuffers;
	// 将缓冲区绑定到渲染管线上
	pCBuffers[0]->BindPS(deviceContext);
	pCBuffers[1]->BindPS(deviceContext);
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
