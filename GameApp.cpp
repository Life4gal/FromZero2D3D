#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance)
	:
	D3DApp(hInstance),
	m_IndexCount(),
	m_VSConstantBuffer(),
	m_PSConstantBuffer(),
	m_DirLight(),
	m_PointLight(),
	m_SpotLight(),
	m_IsWireframeMode()
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

	// 初始化鼠标，键盘不需要
	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(Mouse::MODE_ABSOLUTE);
	
	return true;
}

void GameApp::OnResize()
{
	assert(m_pd2dFactory);
	assert(m_pdwriteFactory);
	// 释放D2D的相关资源
	/*
		在这里D2D的相关资源需要在D3D相关资源释放前先行释放掉，
		然后在D3D重设后备缓冲区后重新创建D2D渲染目标。
		至于D2D后续的相关资源也需要重新创建好来
	 */
	m_pColorBrush.Reset();
	m_pd2dRenderTarget.Reset();
	
	D3DApp::OnResize();

	// 为D2D创建DXGI表面渲染目标
	ComPtr<IDXGISurface> surface;
	HR(m_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(surface.GetAddressOf())));
	/*
		typedef struct D2D1_RENDER_TARGET_PROPERTIES
		{
		    D2D1_RENDER_TARGET_TYPE type;   // 渲染目标类型枚举值
		    D2D1_PIXEL_FORMAT pixelFormat;  
		    FLOAT dpiX;                     // X方向每英寸像素点数，设为0.0f使用默认DPI
		    FLOAT dpiY;                     // Y方向每英寸像素点数，设为0.0f使用默认DPI
		    D2D1_RENDER_TARGET_USAGE usage; // 渲染目标用途枚举值
		    D2D1_FEATURE_LEVEL minLevel;    // D2D最小特性等级

		} D2D1_RENDER_TARGET_PROPERTIES;

		typedef struct D2D1_PIXEL_FORMAT
		{
		    DXGI_FORMAT format;             // DXGI格式
		    D2D1_ALPHA_MODE alphaMode;      // 混合模式

		} D2D1_PIXEL_FORMAT;
	 */
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);
	/*
		HRESULT ID2D1Factory::CreateDxgiSurfaceRenderTarget(
		    IDXGISurface *dxgiSurface,          // [In]DXGI表面
		    const D2D1_RENDER_TARGET_PROPERTIES *renderTargetProperties,    // [In]D2D渲染目标属性
		    ID2D1RenderTarget **renderTarget    // [Out]得到的D2D渲染目标
		);
	 */
	const HRESULT hr = m_pd2dFactory->CreateDxgiSurfaceRenderTarget(surface.Get(), &props, m_pd2dRenderTarget.GetAddressOf());
	surface.Reset();

	if (hr == E_NOINTERFACE)
	{
		OutputDebugStringW(
			L"\n警告：Direct2D与Direct3D互操作性功能受限，你将无法看到文本信息。现提供下述可选方法：\n"
			L"1. 对于Win7系统，需要更新至Win7 SP1，并安装KB2670838补丁以支持Direct2D显示。\n"
			L"2. 自行完成Direct3D 10.1与Direct2D的交互。详情参阅："
			L"https://docs.microsoft.com/zh-cn/windows/desktop/Direct2D/direct2d-and-direct3d-interoperation-overview""\n"
			L"3. 使用别的字体库，比如FreeType。\n\n"
		);
	}
	else if (hr == S_OK)
	{
		// 创建固定颜色刷和文本格式
		/*
			HRESULT ID2D1RenderTarget::CreateSolidColorBrush(
			    const D2D1_COLOR_F &color,  // [In]颜色
			    ID2D1SolidColorBrush **solidColorBrush // [Out]输出的颜色刷
			);

			这里会默认指定Alpha值为1.0
			D2D1_COLOR_F是一个包含r,g,b,a浮点数的结构体，
			但其实还有一种办法可以指定颜色，
			就是利用它的继承类D2D1::ColorF中的构造函数，
			以及D2D1::ColorF::Enum枚举类型来指定要使用的颜色
		 */
		HR(m_pd2dRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::White),
			m_pColorBrush.GetAddressOf())
		);
		/*
			HRESULT IDWriteFactory::CreateTextFormat(
			    const WCHAR * fontFamilyName,           // [In]字体系列名称
			    IDWriteFontCollection * fontCollection, // [In]通常用nullptr来表示使用系统字体集合 
			    DWRITE_FONT_WEIGHT  fontWeight,         // [In]字体粗细程度枚举值
			    DWRITE_FONT_STYLE  fontStyle,           // [In]字体样式枚举值
			    DWRITE_FONT_STRETCH  fontStretch,       // [In]字体拉伸程度枚举值
			    FLOAT  fontSize,                        // [In]字体大小
			    const WCHAR * localeName,               // [In]区域名称
			    IDWriteTextFormat ** textFormat);       // [Out]创建的文本格式

			字体样式如下：
			枚举值						样式
			DWRITE_FONT_STYLE_NORMAL	默认
			DWRITE_FONT_STYLE_OBLIQUE	斜体
			DWRITE_FONT_STYLE_ITALIC	意大利体
		*/
		HR(m_pdwriteFactory->CreateTextFormat(L"微软雅黑", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20, L"zh-cn",
			m_pTextFormat.GetAddressOf())
		);
	}
	else
	{
		// 报告异常问题
		assert(m_pd2dRenderTarget);
	}
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
	phi -= 0.001f, theta += 0.0015f;
	// 物体绕自身Y轴逆时针旋转, 物体中心移动到(3, 0, 0), 物体绕世界Y轴顺时针旋转
	const XMMATRIX W = XMMatrixRotationY(phi) * XMMatrixTranslation(3.0f, 0.0f, 0.0f) * XMMatrixRotationY(theta);
	m_VSConstantBuffer.world = XMMatrixTranspose(W);
	m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, W);	// 两次转置可以抵消

	// 键盘切换灯光类型
	const Keyboard::State kb_state = m_pKeyboard->GetState();
	m_KeyboardTracker.Update(kb_state);
	
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D1))
	{
		m_PSConstantBuffer.dirLight = m_DirLight;
		m_PSConstantBuffer.pointLight = PointLight();
		m_PSConstantBuffer.spotLight = SpotLight();
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D2))
	{
		m_PSConstantBuffer.dirLight = DirectionalLight();
		m_PSConstantBuffer.pointLight = m_PointLight;
		m_PSConstantBuffer.spotLight = SpotLight();
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D3))
	{
		m_PSConstantBuffer.dirLight = DirectionalLight();
		m_PSConstantBuffer.pointLight = PointLight();
		m_PSConstantBuffer.spotLight = m_SpotLight;
	}

	// 键盘切换模型类型
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::Q))
	{
		const Geometry::MeshData<VertexPosNormalColor> meshData = Geometry::CreateBox<VertexPosNormalColor>();
		ResetMesh(meshData);
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::W))
	{
		const Geometry::MeshData<VertexPosNormalColor> meshData = Geometry::CreateSphere<VertexPosNormalColor>();
		ResetMesh(meshData);
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::E))
	{
		const Geometry::MeshData<VertexPosNormalColor> meshData = Geometry::CreateCylinder<VertexPosNormalColor>();
		ResetMesh(meshData);
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::R))
	{
		const Geometry::MeshData<VertexPosNormalColor> meshData = Geometry::CreateCone<VertexPosNormalColor>();
		ResetMesh(meshData);
	}
	// 键盘切换光栅化状态
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::S))
	{
		m_IsWireframeMode = !m_IsWireframeMode;
		m_pd3dImmediateContext->RSSetState(m_IsWireframeMode ? m_pRSWireframe.Get() : nullptr);
	}
	
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
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
	/*
		让指向资源的指针无效并重新启用GPU对该资源的访问权限
		void ID3D11DeviceContext::Unmap(
		    ID3D11Resource *pResource,      // [In]包含ID3D11Resource接口的资源对象
		    UINT           Subresource      // [In]缓冲区资源填0
		);
	 */
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[0].Get(), 0);

	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), Colors::Black);
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 绘制几何模型
	m_pd3dImmediateContext->DrawIndexed(m_IndexCount, 0, 0);

	// 绘制Direct2D部分
	//
	if (m_pd2dRenderTarget != nullptr)
	{
		m_pd2dRenderTarget->BeginDraw();
		std::wstring textStr = 
			L"切换灯光类型: 1-平行光 2-点光 3-聚光灯\n"
			L"切换模型: Q-立方体 W-球体 E-圆柱体 R-圆锥体\n"
			L"S-切换模式 当前模式: ";
		if (m_IsWireframeMode)
			textStr += L"线框模式";
		else
			textStr += L"面模式";
		/*
			void ID2D1RenderTarget::DrawTextW(
			    WCHAR *string,                      // [In]要输出的文本
			    UINT stringLength,                  // [In]文本长度，用wcslen函数或者wstring::length方法获取即可
			    IDWriteTextFormat *textFormat,      // [In]文本格式
			    const D2D1_RECT_F &layoutRect,      // [In]布局区域
			    ID2D1Brush *defaultForegroundBrush, // [In]使用的前景刷
			    D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_NONE,
			    DWRITE_MEASURING_MODE measuringMode = DWRITE_MEASURING_MODE_NATURAL);
		 */
		m_pd2dRenderTarget->DrawTextW(textStr.c_str(), static_cast<UINT32>(textStr.size()), m_pTextFormat.Get(),
			D2D1_RECT_F{ 0.0f, 0.0f, 600.0f, 200.0f }, m_pColorBrush.Get());
		HR(m_pd2dRenderTarget->EndDraw());
	}
	
	HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitEffect()
{
	ComPtr<ID3DBlob> blob;

	// 创建顶点着色器
	HR(CreateShaderFromFile(L"HLSL\\Light_VS.cso", L"HLSL\\Light_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));
	// 创建并绑定顶点布局
	HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalColor::inputLayout, ARRAYSIZE(VertexPosNormalColor::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

	// 创建像素着色器
	HR(CreateShaderFromFile(L"HLSL\\Light_PS.cso", L"HLSL\\Light_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));

	return true;
}

bool GameApp::InitResource()
{
	// ******************
	// 初始化网格模型
	//
	const Geometry::MeshData<VertexPosNormalColor> meshData = Geometry::CreateBox<VertexPosNormalColor>();
	ResetMesh(meshData);

	// ******************
	// 设置常量缓冲区描述
	//
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(VSConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 新建用于VS和PS的常量缓冲区
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(PSConstantBuffer);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf()));

	// ******************
	// 初始化默认光照
	//
	//	float4 颜色向量(RGBA)
	//	ambient  
	//	diffuse 
	//	specular
	// 
	// 方向光
	// 只射红光
	m_DirLight.ambient = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	m_DirLight.diffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	m_DirLight.specular = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	m_DirLight.direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
	// 点光
	// 只射绿光
	m_PointLight.ambient = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	m_PointLight.diffuse = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	m_PointLight.specular = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	m_PointLight.position = XMFLOAT3(0.0f, 0.0f, -10.0f);
	m_PointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_PointLight.range = 25.0f;
	// 聚光灯
	// 只射蓝光
	m_SpotLight.ambient = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	m_SpotLight.diffuse = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	m_SpotLight.specular = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	m_SpotLight.position = XMFLOAT3(0.0f, 0.0f, -5.0f);
	m_SpotLight.direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_SpotLight.att = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_SpotLight.spot = 12.0f;
	m_SpotLight.range = 10000.0f;
	
	// 初始化常量缓冲区的值
	/*
		现在需要利用mBuffer结构体变量用于更新常量缓冲区，
		其中view和proj矩阵需要预先进行一次转置以抵消HLSL列主矩阵的转置，
		至于world矩阵已经是单位矩阵就不需要了
	 */
	 // 初始化用于VS的常量缓冲区的值
	m_VSConstantBuffer.world = XMMatrixIdentity();		// 单位矩阵的转置是它本身
	/*
		若已知Q为摄像机的位置，T为摄像机对准的观察目标点，j为世界空间“向上”方向的单位向量。
		以平面xOz作为场景中的“地平面”，并以世界空间的y轴作为摄像机“向上”的方向。因此，j = (0,1,0)仅是平行于世界空间中y轴的一个单位向量，虚拟摄像机的观察方向为：

		||vector||表示取向量的模长,做分母用于取单位向量
		虚拟摄像机局部空间的z轴:
		w = T−Q / ||T−Q||			
		虚拟摄像机局部空间的x轴：
		u = j×w / ||j×w||		// 这里用的是叉积(外积)
		虚拟摄像机局部空间的y轴：
		v = w×u
		因为w和u是互相正交的单位向量，所以v也必为单位向量。因此我们也无须对向量v进行规范化处理了。
		
		针对上述计算观察矩阵的处理流程提供了以下函数：
		// 观察矩阵
		XMMATRIX XMMatrixLookAtLH(  // 输出视图变换矩阵V
		    FXMVECTOR EyePosition,      // 输入摄影机坐标
		    FXMVECTOR FocusPosition,    // 输入摄影机焦点坐标
		    FXMVECTOR UpDirection);     // 输入摄影机上朝向坐标
	 */
	m_VSConstantBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	));
	/*
		// 透视投影矩阵
		XMMATRIX XMMatrixPerspectiveFovLH( // 返回投影矩阵
		    FLOAT FovAngleY,                   // 中心垂直弧度
		    FLOAT AspectRatio,                 // 宽高比
		    FLOAT NearZ,                       // 近平面距离
		    FLOAT FarZ);                       // 远平面距离
	 */
	m_VSConstantBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));
	m_VSConstantBuffer.worldInvTranspose = XMMatrixIdentity();

	// 初始化用于PS的常量缓冲区的值
	m_PSConstantBuffer.material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_PSConstantBuffer.material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_PSConstantBuffer.material.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 5.0f /* 镜面反射系数 */);
	// 使用默认平行光
	m_PSConstantBuffer.dirLight = m_DirLight;
	// 注意不要忘记设置此处的观察位置，否则高亮部分会有问题
	m_PSConstantBuffer.eyePos = XMFLOAT4(0.0f, 0.0f, -10.0f, 0.0f);

	// 更新PS常量缓冲区资源
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);
	// ******************
	// 初始化光栅化状态
	//
	/*
		typedef struct D3D11_RASTERIZER_DESC
		{
		    D3D11_FILL_MODE FillMode;          // 填充模式
		    D3D11_CULL_MODE CullMode;          // 裁剪模式
		    BOOL FrontCounterClockwise;        // 是否三角形顶点按逆时针排布时为正面
		    INT DepthBias;                     // 忽略
		    FLOAT DepthBiasClamp;              // 忽略
		    FLOAT SlopeScaledDepthBias;        // 忽略
		    BOOL DepthClipEnable;              // 是否允许深度测试将范围外的像素进行裁剪，默认TRUE
		    BOOL ScissorEnable;                // 是否允许指定矩形范围的裁剪，若TRUE，则需要在RSSetScissor设置像素保留的矩形区域
		    BOOL MultisampleEnable;            // 是否允许多重采样
		    BOOL AntialiasedLineEnable;        // 是否允许反走样线，仅当多重采样为FALSE时才有效
		} 	D3D11_RASTERIZER_DESC;

		枚举值								含义
		D3D11_FILL_WIREFRAME = 2			线框填充方式
		D3D11_FILL_SOLID = 3				面填充方式

		D3D11_CULL_NONE = 1					无背面裁剪，即三角形无论处在视野的正面还是背面都能看到
		D3D11_CULL_FRONT = 2				对处在视野正面的三角形进行裁剪
		D3D11_CULL_BACK = 3					对处在视野背面的三角形进行裁剪

		默认光栅化状态如下：
			FillMode = D3D11_FILL_SOLID;
			CullMode = D3D11_CULL_BACK;
			FrontCounterClockwise = FALSE;
			DepthBias = 0;
			SlopeScaledDepthBias = 0.0f;
			DepthBiasClamp = 0.0f;
			DepthClipEnable	= TRUE;
			ScissorEnable = FALSE;
			MultisampleEnable = FALSE;
			AntialiasedLineEnable = FALSE;
	 */
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;
	/*
		HRESULT ID3D11Device::CreateRasterizerState( 
			const D3D11_RASTERIZER_DESC *pRasterizerDesc,    // [In]光栅化状态描述
			ID3D11RasterizerState **ppRasterizerState) = 0;  // [Out]输出光栅化状态
	 */
	HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireframe.GetAddressOf()));

	// ******************
	// 给渲染管线各个阶段绑定好所需资源
	//

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
	// VS常量缓冲区对应HLSL寄存于b0的常量缓冲区
	m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
	// PS常量缓冲区对应HLSL寄存于b1的常量缓冲区
	m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

	// ******************
	// 设置调试对象名
	//
	D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosNormalTexLayout");
	D3D11SetDebugObjectName(m_pConstantBuffers[0].Get(), "VSConstantBuffer");
	D3D11SetDebugObjectName(m_pConstantBuffers[1].Get(), "PSConstantBuffer");
	D3D11SetDebugObjectName(m_pVertexShader.Get(), "Light_VS");
	D3D11SetDebugObjectName(m_pPixelShader.Get(), "Light_PS");

	return true;
}

bool GameApp::ResetMesh(const Geometry::MeshData<VertexPosNormalColor>& meshData)
{
	// 释放旧资源
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	// 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = static_cast<UINT>(meshData.vertexVec.size()) * sizeof(VertexPosNormalColor);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

	// 输入装配阶段的顶点缓冲区设置
	UINT stride = sizeof(VertexPosNormalColor);	// 跨越字节数
	UINT offset = 0;							// 起始偏移量

	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);

	// 设置索引缓冲区描述
	m_IndexCount = static_cast<UINT>(meshData.indexVec.size());
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = m_IndexCount * sizeof(DWORD);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = meshData.indexVec.data();
	HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
	// 输入装配阶段的索引缓冲区设置
	m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// 设置调试对象名
	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
	D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");

	return true;
}
