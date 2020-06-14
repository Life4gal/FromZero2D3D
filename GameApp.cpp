#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace DirectX;

const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosColor::inputLayout[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	if (!InitEffect())
		return false;

	if (!InitResource())
		return false;

	return true;
}

void GameApp::OnResize()
{
	D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{
	/*
		在创建资源的时候指定Usage为D3D11_USAGE_DYNAMIC、CPUAccessFlags为D3D11_CPU_ACCESS_WRITE，
		允许常量缓冲区从CPU写入，首先通过ID3D11DeviceContext::Map方法获取内存映射，
		然后再更新到映射好的内存区域，最后通过ID3D11DeviceContext::Unmap方法解除占用。

		适合于需要频繁更新，如每几帧更新一次，或每帧更新一次或多次的资源。
	 */
	static float phi = 0.0f, theta = 0.0f;
	phi += 0.001f, theta += 0.0015f;
	m_CBuffer.world = XMMatrixTranspose(XMMatrixRotationX(phi) * XMMatrixRotationY(theta));
	// 更新常量缓冲区，让立方体转起来
	D3D11_MAPPED_SUBRESOURCE mappedData;
	/*
	获取指向缓冲区中数据的指针并拒绝GPU对该缓冲区的访问
	HRESULT ID3D11DeviceContext::Map(
	    ID3D11Resource           *pResource,          // [In]包含ID3D11Resource接口的资源对象
	    UINT                     Subresource,         // [In]缓冲区资源填0
	    D3D11_MAP                MapType,             // [In]D3D11_MAP枚举值，指定读写相关操作
	    UINT                     MapFlags,            // [In]填0，CPU需要等待GPU使用完毕当前缓冲区
	    D3D11_MAPPED_SUBRESOURCE *pMappedResource     // [Out]获取到的已经映射到缓冲区的内存
	);

	D3D11_MAP成员					含义
	D3D11_MAP_READ					映射到内存的资源用于读取。该资源在创建的时候必须绑定了D3D11_CPU_ACCESS_READ标签
	D3D11_MAP_WRITE					映射到内存的资源用于写入。该资源在创建的时候必须绑定了D3D11_CPU_ACCESS_WRITE标签
	D3D11_MAP_READ_WRITE			映射到内存的资源用于读写。该资源在创建的时候必须绑定了D3D11_CPU_ACCESS_READ和D3D11_CPU_ACCESS_WRITE标签
	D3D11_MAP_WRITE_DISCARD			映射到内存的资源用于写入，之前的资源数据将会被抛弃。该资源在创建的时候必须绑定了D3D11_CPU_ACCESS_WRITE和D3D11_USAGE_DYNAMIC标签
	D3D11_MAP_WRITE_NO_OVERWRITE	映射到内存的资源用于写入，但不能复写已经存在的资源。该枚举值只能用于顶点/索引缓冲区。
									该资源在创建的时候需要有D3D11_CPU_ACCESS_WRITE标签，在Direct3D 11不能用于设置了D3D11_BIND_CONSTANT_BUFFER标签的资源，但在11.1后可以。具体可以查阅MSDN文档
	 */
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(m_CBuffer), &m_CBuffer, sizeof(m_CBuffer));
	/*
	让指向资源的指针无效并重新启用GPU对该资源的访问权限
	void ID3D11DeviceContext::Unmap(
	    ID3D11Resource *pResource,      // [In]包含ID3D11Resource接口的资源对象
	    UINT           Subresource      // [In]缓冲区资源填0
	);
	 */
	m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);
	constexpr static float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	// RGBA = (0,0,0,255)
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), black);
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 绘制立方体
	m_pd3dImmediateContext->DrawIndexed(36, 0, 0);
	HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitEffect()
{
	ComPtr<ID3DBlob> blob;

	// 创建顶点着色器
	HR(CreateShaderFromFile(L"HLSL\\Cube_VS.cso", L"HLSL\\Cube_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));
	// 创建顶点布局
	HR(m_pd3dDevice->CreateInputLayout(VertexPosColor::inputLayout, ARRAYSIZE(VertexPosColor::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

	// 创建像素着色器
	HR(CreateShaderFromFile(L"HLSL\\Cube_PS.cso", L"HLSL\\Cube_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));

	return true;
}

bool GameApp::InitResource()
{
	// 注意顶点的给出顺序应当按顺时针排布
	// ******************
	// 设置立方体顶点
	//    5________ 6
	//    /|      /|
	//   /_|_____/ |
	//  1|4|_ _ 2|_|7
	//   | /     | /
	//   |/______|/
	//  0       3
	VertexPosColor vertices[] =
	{
		{ XMFLOAT3(-2.0f, -2.0f, -2.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-2.0f, 2.0f, -2.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(2.0f, 2.0f, -2.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(2.0f, -2.0f, -2.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-2.0f, -2.0f, 2.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-2.0f, 2.0f, 2.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(2.0f, 2.0f, 2.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(2.0f, -2.0f, 2.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }
	};
	// 设置顶点缓冲区描述
	/*
		 typedef struct D3D11_BUFFER_DESC
		{
			UINT ByteWidth;             // 数据字节数
			D3D11_USAGE Usage;          // CPU和GPU的读写权限相关
			UINT BindFlags;             // 缓冲区类型的标志
			UINT CPUAccessFlags;        // CPU读写权限的指定
			UINT MiscFlags;             // 忽略
			UINT StructureByteStride;   // 忽略
		} 	D3D11_BUFFER_DESC;
	 */
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	/*
		在这里需要详细讲述一下D3D11_USAGE枚举类型对应的读写关系：
								CPU读	CPU写	GPU读	GPU写
		D3D11_USAGE_DEFAULT						√		√
		D3D11_USAGE_IMMUTABLE					√	
		D3D11_USAGE_DYNAMIC				√		√	
		D3D11_USAGE_STAGING		√		√		√		√
	 */
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof vertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	/*
	typedef struct D3D11_SUBRESOURCE_DATA
	{
	    const void *pSysMem;        // 用于初始化的数据
	    UINT SysMemPitch;           // 忽略
	    UINT SysMemSlicePitch;      // 忽略
	} 	D3D11_SUBRESOURCE_DATA;
	 */
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

	// ******************
	// 索引数组
	//
	DWORD indices[] = 
	{
		// 正面
		0, 1, 2,
		2, 3, 0,
		// 左面
		4, 5, 1,
		1, 0, 4,
		// 顶面
		1, 5, 6,
		6, 2, 1,
		// 背面
		7, 6, 5,
		5, 4, 7,
		// 右面
		3, 2, 6,
		6, 7, 3,
		// 底面
		4, 0, 3,
		3, 7, 4
	};
	// 设置索引缓冲区描述
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(indices);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = indices;
	HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
	// 输入装配阶段的索引缓冲区设置
	m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// ******************
	// 设置常量缓冲区描述
	//
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 新建常量缓冲区，不使用初始数据
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffer.GetAddressOf()));

	// 初始化常量缓冲区的值
	/*
		现在需要利用mBuffer结构体变量用于更新常量缓冲区，
		其中view和proj矩阵需要预先进行一次转置以抵消HLSL列主矩阵的转置，
		至于world矩阵已经是单位矩阵就不需要了
	 */
	m_CBuffer.world = XMMatrixIdentity();	// 单位矩阵的转置是它本身
	m_CBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	));
	m_CBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));

	// ******************
	// 给渲染管线各个阶段绑定好所需资源
	//

	// 输入装配阶段的顶点缓冲区设置
	UINT stride = sizeof(VertexPosColor);	// 跨越字节数
	UINT offset = 0;						// 起始偏移量

	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	// 设置图元类型，设定输入布局
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
	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
	// 将着色器绑定到渲染管线
	m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
	// 将更新好的常量缓冲区绑定到顶点着色器
	m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
	
	m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

	// ******************
	// 设置调试对象名
	//
	D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosColorLayout");
	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
	D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");
	D3D11SetDebugObjectName(m_pConstantBuffer.Get(), "ConstantBuffer");
	D3D11SetDebugObjectName(m_pVertexShader.Get(), "Cube_VS");
	D3D11SetDebugObjectName(m_pPixelShader.Get(), "Cube_PS");

	return true;
}