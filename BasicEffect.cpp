#include "BasicEffect.h"

using namespace DirectX;

//
// BasicEffect::Impl 需要先于BasicEffect的定义
//

class BasicEffect::Impl
{
public:
	std::unique_ptr<EffectHelper> m_pEffectHelper;

	std::shared_ptr<IEffectPass> m_pCurrEffectPass;

	ComPtr<ID3D11InputLayout> m_pInstancePosNormalTexLayout;
	ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexLayout;
	ComPtr<ID3D11InputLayout> m_pInstancePosNormalTangentTexLayout;
	ComPtr<ID3D11InputLayout> m_pVertexPosNormalTangentTexLayout;

	XMFLOAT4X4 m_world{};
	XMFLOAT4X4 m_view{};
	XMFLOAT4X4 m_proj{};
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

	if (!RenderStates::IsInit())
		throw std::exception("RenderStates need to be initialized first!");

	m_pImpl->m_pEffectHelper = std::make_unique<EffectHelper>();
	
	// 实例输入布局
	/*
		因为DXGI_FORMAT一次最多仅能够表达128位(16字节)数据，
		在对应矩阵的语义时，需要重复描述4次，区别在于语义索引为0-3.
	 */
	D3D11_INPUT_ELEMENT_DESC basicInstLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "World", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "World", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "World", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "World", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WorldInvTranspose", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WorldInvTranspose", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WorldInvTranspose", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WorldInvTranspose", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	D3D11_INPUT_ELEMENT_DESC normalMapInstLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "World", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "World", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "World", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "World", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WorldInvTranspose", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WorldInvTranspose", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WorldInvTranspose", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WorldInvTranspose", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};
	
	ComPtr<ID3DBlob> blob;

	// ******************
	// 创建顶点着色器
	//

	HR(CreateShaderFromFile(L"HLSL\\BasicInstance_VS.cso", L"HLSL\\BasicInstance_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("BasicInstance_VS", device, blob.Get()));
	// 创建顶点布局
	HR(device->CreateInputLayout(basicInstLayout, ARRAYSIZE(basicInstLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pImpl->m_pInstancePosNormalTexLayout.GetAddressOf()));


	HR(CreateShaderFromFile(L"HLSL\\BasicObject_VS.cso", L"HLSL\\BasicObject_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("BasicObject_VS", device, blob.Get()));
	// 创建顶点布局
	HR(device->CreateInputLayout(VertexPosNormalTex::InputLayout, ARRAYSIZE(VertexPosNormalTex::InputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pImpl->m_pVertexPosNormalTexLayout.GetAddressOf()));

	HR(CreateShaderFromFile(L"HLSL\\NormalMapInstance_VS.cso", L"HLSL\\NormalMapInstance_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("NormalMapInstance_VS", device, blob.Get()));
	// 创建顶点布局
	HR(device->CreateInputLayout(normalMapInstLayout, ARRAYSIZE(normalMapInstLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pImpl->m_pInstancePosNormalTangentTexLayout.GetAddressOf()));

	HR(CreateShaderFromFile(L"HLSL\\NormalMapObject_VS.cso", L"HLSL\\NormalMapObject_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("NormalMapObject_VS", device, blob.Get()));
	// 创建顶点布局
	HR(device->CreateInputLayout(VertexPosNormalTangentTex::InputLayout, ARRAYSIZE(VertexPosNormalTangentTex::InputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pImpl->m_pVertexPosNormalTangentTexLayout.GetAddressOf()));

	// ******************
	// 创建像素着色器
	//

	HR(CreateShaderFromFile(L"HLSL\\Basic_PS.cso", L"HLSL\\Basic_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("Basic_PS", device, blob.Get()));

	HR(CreateShaderFromFile(L"HLSL\\NormalMap_PS.cso", L"HLSL\\NormalMap_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pImpl->m_pEffectHelper->AddShader("NormalMap_PS", device, blob.Get()));

	// ******************
	// 创建通道
	//
	EffectPassDesc passDesc;
	passDesc.nameVS = "BasicObject_VS";
	passDesc.namePS = "Basic_PS";
	m_pImpl->m_pEffectHelper->AddEffectPass("BasicObject", device, &passDesc);
	passDesc.nameVS = "BasicInstance_VS";
	passDesc.namePS = "Basic_PS";
	m_pImpl->m_pEffectHelper->AddEffectPass("BasicInstance", device, &passDesc);
	passDesc.nameVS = "NormalMapObject_VS";
	passDesc.namePS = "NormalMap_PS";
	m_pImpl->m_pEffectHelper->AddEffectPass("NormalMapObject", device, &passDesc);
	passDesc.nameVS = "NormalMapInstance_VS";
	passDesc.namePS = "NormalMap_PS";
	m_pImpl->m_pEffectHelper->AddEffectPass("NormalMapInstance", device, &passDesc);

	m_pImpl->m_pEffectHelper->SetSamplerStateByName("g_Sam", RenderStates::SSLinearWrap.Get());
	m_pImpl->m_pEffectHelper->SetSamplerStateByName("g_SamShadow", RenderStates::SSShadow.Get());

	// 设置调试对象名
	D3D11SetDebugObjectName(m_pImpl->m_pInstancePosNormalTexLayout.Get(), "BasicEffect.InstancePosNormalTexLayout");
	D3D11SetDebugObjectName(m_pImpl->m_pVertexPosNormalTexLayout.Get(), "BasicEffect.VertexPosNormalTexLayout");
	D3D11SetDebugObjectName(m_pImpl->m_pInstancePosNormalTangentTexLayout.Get(), "BasicEffect.InstancePosNormalTangentTexLayout");
	D3D11SetDebugObjectName(m_pImpl->m_pVertexPosNormalTangentTexLayout.Get(), "BasicEffect.VertexPosNormalTangentTexLayout");
	m_pImpl->m_pEffectHelper->SetDebugObjectName("BasicEffect");

	return true;
}

void BasicEffect::SetRenderDefault(ID3D11DeviceContext* deviceContext, const RenderType type) const
{
	if (type == RenderType::RenderInstance)
	{
		deviceContext->IASetInputLayout(m_pImpl->m_pInstancePosNormalTexLayout.Get());
		m_pImpl->m_pCurrEffectPass = m_pImpl->m_pEffectHelper->GetEffectPass("BasicInstance");
	}
	else
	{
		deviceContext->IASetInputLayout(m_pImpl->m_pVertexPosNormalTexLayout.Get());
		m_pImpl->m_pCurrEffectPass = m_pImpl->m_pEffectHelper->GetEffectPass("BasicObject");
	}
	
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
}

void BasicEffect::SetRenderWithNormalMap(ID3D11DeviceContext* deviceContext, RenderType type) const
{
	if (type == RenderType::RenderInstance)
	{
		deviceContext->IASetInputLayout(m_pImpl->m_pInstancePosNormalTangentTexLayout.Get());
		m_pImpl->m_pCurrEffectPass = m_pImpl->m_pEffectHelper->GetEffectPass("NormalMapInstance");
	}
	else
	{
		deviceContext->IASetInputLayout(m_pImpl->m_pVertexPosNormalTangentTexLayout.Get());
		m_pImpl->m_pCurrEffectPass = m_pImpl->m_pEffectHelper->GetEffectPass("NormalMapObject");
	}

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void XM_CALLCONV BasicEffect::SetWorldMatrix(FXMMATRIX world) const
{
	XMStoreFloat4x4(&m_pImpl->m_world, world);
}

void XM_CALLCONV BasicEffect::SetViewMatrix(FXMMATRIX view) const
{
	XMStoreFloat4x4(&m_pImpl->m_view, view);
}

void XM_CALLCONV BasicEffect::SetProjMatrix(FXMMATRIX proj) const
{
	XMStoreFloat4x4(&m_pImpl->m_proj, proj);
}

void XM_CALLCONV BasicEffect::SetShadowTransformMatrix(FXMMATRIX shadow) const
{
	XMMATRIX shadowTransform = XMMatrixTranspose(shadow);
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ShadowTransform")->SetFloatMatrix(4, 4, reinterpret_cast<const FLOAT*>(&shadowTransform));
}

void BasicEffect::SetDirLight(const size_t position, const DirectionalLight& dirLight) const
{
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_DirLight")->SetRaw(&dirLight, static_cast<UINT>(position) * sizeof(dirLight), sizeof(dirLight));
}

void BasicEffect::SetPointLight(const size_t position, const PointLight& pointLight) const
{
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_PointLight")->SetRaw(&pointLight, static_cast<UINT>(position) * sizeof(pointLight), sizeof(pointLight));
}

void BasicEffect::SetSpotLight(const size_t position, const SpotLight& spotLight) const
{
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_SpotLight")->SetRaw(&spotLight, static_cast<UINT>(position) * sizeof(spotLight), sizeof(spotLight));
}

void BasicEffect::SetMaterial(const Material& material) const
{
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_Material")->SetRaw(&material);
}

void BasicEffect::SetTextureUsed(const bool isUsed) const
{
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_TextureUsed")->SetSInt(isUsed);
}

void BasicEffect::SetShadowEnabled(const bool enabled) const
{
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_EnableShadow")->SetSInt(enabled);
}

void BasicEffect::SetTextureDiffuse(ID3D11ShaderResourceView* textureDiffuse) const
{
	m_pImpl->m_pEffectHelper->SetShaderResourceByName("g_DiffuseMap", textureDiffuse);
}

void BasicEffect::SetTextureNormalMap(ID3D11ShaderResourceView* textureNormalMap) const
{
	m_pImpl->m_pEffectHelper->SetShaderResourceByName("g_NormalMap", textureNormalMap);
}

void BasicEffect::SetTextureShadowMap(ID3D11ShaderResourceView* textureShadowMap) const
{
	m_pImpl->m_pEffectHelper->SetShaderResourceByName("g_ShadowMap", textureShadowMap);
}

void BasicEffect::SetTextureCube(ID3D11ShaderResourceView* textureCube) const
{
	m_pImpl->m_pEffectHelper->SetShaderResourceByName("g_TexCube", textureCube);
}

void XM_CALLCONV BasicEffect::SetEyePos(FXMVECTOR eyePos) const
{
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_EyePosW")->SetFloatVector(3, reinterpret_cast<const float*>(&eyePos));
}

void BasicEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	XMMATRIX world = XMLoadFloat4x4(&m_pImpl->m_world);
	XMMATRIX view = XMLoadFloat4x4(&m_pImpl->m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_pImpl->m_proj);

	XMMATRIX worldViewPrjMatrix = XMMatrixTranspose(world * view * proj);
	XMMATRIX worldInvTranspose = XMMatrixTranspose(InverseTranspose(world));

	world = XMMatrixTranspose(world);
	view = XMMatrixTranspose(view);
	proj = XMMatrixTranspose(proj);

	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_World")->SetFloatMatrix(4, 4, reinterpret_cast<const float*>(&world));
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_View")->SetFloatMatrix(4, 4, reinterpret_cast<const float*>(&view));
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_Proj")->SetFloatMatrix(4, 4, reinterpret_cast<const float*>(&proj));
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldViewProj")->SetFloatMatrix(4, 4, reinterpret_cast<const float*>(&worldViewPrjMatrix));
	m_pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldInvTranspose")->SetFloatMatrix(4, 4, reinterpret_cast<const float*>(&worldInvTranspose));

	if (m_pImpl->m_pCurrEffectPass)
	{
		m_pImpl->m_pCurrEffectPass->Apply(deviceContext);
	}	
}
