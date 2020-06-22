#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance)
	:
	D3DApp(hInstance),
	m_CBFrame(),
	m_CBOnResize(),
	m_CBRarely(),
	m_CameraMode(CameraMode::FirstPerson)
{
}

GameApp::~GameApp() = default;

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
	m_pMouse->SetMode(Mouse::MODE_RELATIVE);
	
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

	// 摄像机变更显示
	if (m_pCamera != nullptr)
	{
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, static_cast<float>(m_ClientWidth), static_cast<float>(m_ClientHeight));
		m_CBOnResize.proj = XMMatrixTranspose(m_pCamera->GetProjXM());

		D3D11_MAPPED_SUBRESOURCE mappedData;
		HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
		memcpy_s(mappedData.pData, sizeof(CBChangesOnResize), &m_CBOnResize, sizeof(CBChangesOnResize));
		m_pd3dImmediateContext->Unmap(m_pConstantBuffers[2].Get(), 0);
	}
}

void GameApp::UpdateScene(float dt)
{
	 // 更新鼠标事件，获取相对偏移量
	const Mouse::State mouseState = m_pMouse->GetState();
	
	const Keyboard::State keyState = m_pKeyboard->GetState();
	m_KeyboardTracker.Update(keyState);

	// 获取子类
	auto first_person_camera = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
	auto third_person_camera = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);

	Transform& woodCrateTransform = m_WoodCrate.GetTransform();

	if (m_CameraMode == CameraMode::FirstPerson || m_CameraMode == CameraMode::Free)
	{
		// 第一人称/自由摄像机的操作

		// 方向移动
		if (keyState.IsKeyDown(Keyboard::W))
		{
			if (m_CameraMode == CameraMode::FirstPerson)
				first_person_camera->Walk(dt * 6.0f);
			else
				first_person_camera->MoveForward(dt * 6.0f);
		}
		if (keyState.IsKeyDown(Keyboard::S))
		{
			if (m_CameraMode == CameraMode::FirstPerson)
				first_person_camera->Walk(dt * -6.0f);
			else
				first_person_camera->MoveForward(dt * -6.0f);
		}
		if (keyState.IsKeyDown(Keyboard::A))
			first_person_camera->Strafe(dt * -6.0f);
		if (keyState.IsKeyDown(Keyboard::D))
			first_person_camera->Strafe(dt * 6.0f);

		// 将摄像机位置限制在[-8.9, 8.9]x[-8.9, 8.9]x[0.0, 8.9]的区域内
		// 不允许穿地
		const XMVECTOR adjustedPos = XMVectorClamp(first_person_camera->GetPositionXM(), XMVectorSet(-8.9f, 0.0f, -8.9f, 0.0f), XMVectorReplicate(8.9f));
		first_person_camera->SetPosition(adjustedPos);

		// 仅在第一人称模式移动摄像机的同时移动箱子
		if (m_CameraMode == CameraMode::FirstPerson)
			woodCrateTransform.SetPosition(adjustedPos);
		// 在鼠标没进入窗口前仍为ABSOLUTE模式
		if (mouseState.positionMode == Mouse::MODE_RELATIVE)
		{
			first_person_camera->Pitch(static_cast<float>(mouseState.y) * dt * 2.5f);
			first_person_camera->RotateY(static_cast<float>(mouseState.x) * dt * 2.5f);
		}
	}
	else if (m_CameraMode == CameraMode::ThirdPerson)
	{
		// 第三人称摄像机的操作
		third_person_camera->SetTarget(woodCrateTransform.GetPosition());

		// 绕物体旋转
		third_person_camera->RotateX(static_cast<float>(mouseState.y) * dt * 2.5f);
		third_person_camera->RotateY(static_cast<float>(mouseState.x) * dt * 2.5f);
		third_person_camera->Approach(static_cast<float>(-mouseState.scrollWheelValue) / 120 * 1.0f);
	}

	// 更新观察矩阵
	XMStoreFloat4(&m_CBFrame.eyePos, m_pCamera->GetPositionXM());
	m_CBFrame.view = XMMatrixTranspose(m_pCamera->GetViewXM());

	// 重置滚轮值
	m_pMouse->ResetScrollWheelValue();

	// 摄像机模式切换
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D1) && m_CameraMode != CameraMode::FirstPerson)
	{
		if (!first_person_camera)
		{
			first_person_camera.reset(new FirstPersonCamera);
			first_person_camera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = first_person_camera;
		}

		first_person_camera->LookTo(woodCrateTransform.GetPosition(),
			XMFLOAT3(0.0f, 0.0f, 1.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f));

		m_CameraMode = CameraMode::FirstPerson;
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D2) && m_CameraMode != CameraMode::ThirdPerson)
	{
		if (!third_person_camera)
		{
			third_person_camera.reset(new ThirdPersonCamera);
			third_person_camera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = third_person_camera;
		}
		third_person_camera->SetTarget(woodCrateTransform.GetPosition());
		third_person_camera->SetDistance(8.0f);
		third_person_camera->SetDistanceMinMax(3.0f, 20.0f);

		m_CameraMode = CameraMode::ThirdPerson;
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D3) && m_CameraMode != CameraMode::Free)
	{
		if (!first_person_camera)
		{
			first_person_camera.reset(new FirstPersonCamera);
			first_person_camera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = first_person_camera;
		}
		// 从箱子上方开始
		XMFLOAT3 pos = woodCrateTransform.GetPosition();
		pos.y += 3;
		const XMFLOAT3 to = XMFLOAT3(0.0f, 0.0f, 1.0f);
		const XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		first_person_camera->LookTo(pos, to, up);

		m_CameraMode = CameraMode::Free;
	}

	// 退出程序，这里应向窗口发送销毁信息
	if (keyState.IsKeyDown(Keyboard::Escape))
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);

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

		在创建资源的时候指定Usage为D3D11_USAGE_DYNAMIC、CPUAccessFlags为D3D11_CPU_ACCESS_WRITE，
		允许常量缓冲区从CPU写入，首先通过ID3D11DeviceContext::Map方法获取内存映射，
		然后再更新到映射好的内存区域，最后通过ID3D11DeviceContext::Unmap方法解除占用。
		适合于需要频繁更新，如每几帧更新一次，或每帧更新一次或多次的资源。

	*/
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBChangesEveryFrame), &m_CBFrame, sizeof(CBChangesEveryFrame));
	/*
		让指向资源的指针无效并重新启用GPU对该资源的访问权限
		void ID3D11DeviceContext::Unmap(
			ID3D11Resource *pResource,      // [In]包含ID3D11Resource接口的资源对象
			UINT           Subresource      // [In]缓冲区资源填0
		);
	*/
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), Colors::Black);
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	// 绘制几何模型
	m_WoodCrate.Draw(m_pd3dImmediateContext.Get());
	m_Floor.Draw(m_pd3dImmediateContext.Get());
	for (auto& wall : m_Walls)
	{
		wall.Draw(m_pd3dImmediateContext.Get());
	}		
	
	// 绘制Direct2D部分
	//
	if (m_pd2dRenderTarget != nullptr)
	{
		m_pd2dRenderTarget->BeginDraw();
		std::wstring text = L"切换摄像机模式: 1-第一人称 2-第三人称 3-自由视角\n"
			L"W/S/A/D 前进/后退/左平移/右平移 (第三人称无效)  Esc退出\n"
			L"鼠标移动控制视野 滚轮控制第三人称观察距离\n"
			L"当前模式: ";
		if (m_CameraMode == CameraMode::FirstPerson)
			text += L"第一人称(控制箱子移动)";
		else if (m_CameraMode == CameraMode::ThirdPerson)
			text += L"第三人称";
		else
			text += L"自由视角";
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
		m_pd2dRenderTarget->DrawTextW(text.c_str(), static_cast<UINT32>(text.length()), m_pTextFormat.Get(),
			D2D1_RECT_F{ 0.0f, 0.0f, 600.0f, 200.0f }, m_pColorBrush.Get());
		HR(m_pd2dRenderTarget->EndDraw());
	}
	
	HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitEffect()
{
	ComPtr<ID3DBlob> blob;

	// 创建顶点着色器(2D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_VS_2D.cso", L"HLSL\\Basic_VS_2D.hlsl", "VS_2D", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader2D.GetAddressOf()));
	// 创建顶点布局(2D)
	HR(m_pd3dDevice->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout2D.GetAddressOf()));

	// 创建像素着色器(2D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_PS_2D.cso", L"HLSL\\Basic_PS_2D.hlsl", "PS_2D", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader2D.GetAddressOf()));

	// 创建顶点着色器(3D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_VS_3D.cso", L"HLSL\\Basic_VS_3D.hlsl", "VS_3D", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader3D.GetAddressOf()));
	// 创建顶点布局(3D)
	HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout3D.GetAddressOf()));

	// 创建像素着色器(3D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_PS_3D.cso", L"HLSL\\Basic_PS_3D.hlsl", "PS_3D", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader3D.GetAddressOf()));

	return true;
}

bool GameApp::InitResource()
{
	// ******************
	// 设置常量缓冲区描述
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 新建用于VS和PS的常量缓冲区
	cbd.ByteWidth = sizeof(CBChangesEveryDrawing);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesEveryFrame);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesOnResize);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[2].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesRarely);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[3].GetAddressOf()));
	
	// ******************
	// 初始化游戏对象
	ComPtr<ID3D11ShaderResourceView> texture;
	/*
		DDS是一种图片格式，是DirectDraw Surface的缩写，
		它是DirectX纹理压缩（DirectX Texture Compression，简称DXTC）的产物。由NVIDIA公司开发。
		大部分3D游戏引擎都可以使用DDS格式的图片用作贴图，也可以制作法线贴图。
		
		HRESULT CreateDDSTextureFromFile(
			ID3D11Device* d3dDevice,                // [In]D3D设备
			const wchar_t* szFileName,              // [In]dds图片文件名
			ID3D11Resource** texture,               // [Out]输出一个指向资源接口类的指针，也可以填nullptr
			ID3D11ShaderResourceView** textureView, // [Out]输出一个指向着色器资源视图的指针，也可以填nullptr
			size_t maxsize = 0,                     // [In]忽略
			DDS_ALPHA_MODE* alphaMode = nullptr);  // [In]忽略
	 */
	 // 初始化木箱纹理
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	m_WoodCrate.SetBuffer(m_pd3dDevice.Get(), Geometry::CreateBox());
	m_WoodCrate.SetTexture(texture.Get());

	// 初始化地板
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Floor.SetBuffer(m_pd3dDevice.Get(),
		Geometry::CreatePlane(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(5.0f, 5.0f)));
	m_Floor.SetTexture(texture.Get());
	m_Floor.GetTransform().SetPosition(0.0f, -1.0f, 0.0f);

	// 初始化墙体
	m_Walls.resize(4);
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	// 这里控制墙体四个面的生成
	for (int i = 0; i < 4; ++i)
	{
		m_Walls[i].SetBuffer(m_pd3dDevice.Get(),
			Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 1.5f)));
		Transform& transform = m_Walls[i].GetTransform();
		transform.SetRotation(-XM_PIDIV2, XM_PIDIV2 * static_cast<float>(i), 0.0f);
		transform.SetPosition(i % 2 ? -10.0f * static_cast<float>(i - 2) : 0.0f, 3.0f, i % 2 == 0 ? -10.0f * static_cast<float>(i - 1) : 0.0f);
		m_Walls[i].SetTexture(texture.Get());
	}

	// 初始化采样器状态
	/*
		typedef struct D3D11_SAMPLER_DESC
		{
			D3D11_FILTER Filter;                    // 所选过滤器
			D3D11_TEXTURE_ADDRESS_MODE AddressU;    // U方向寻址模式
			D3D11_TEXTURE_ADDRESS_MODE AddressV;    // V方向寻址模式
			D3D11_TEXTURE_ADDRESS_MODE AddressW;    // W方向寻址模式
			FLOAT MipLODBias;   // mipmap等级偏移值，最终算出的mipmap等级会加上该偏移值
			UINT MaxAnisotropy;                     // 最大各向异性等级(1-16)
			D3D11_COMPARISON_FUNC ComparisonFunc;   // 这节不讨论
			FLOAT BorderColor[ 4 ];     // 边界外的颜色，使用D3D11_TEXTURE_BORDER_COLOR时需要指定
			FLOAT MinLOD;   // 若mipmap等级低于MinLOD，则使用等级MinLOD。最小允许设为0
			FLOAT MaxLOD;   // 若mipmap等级高于MaxLOD，则使用等级MaxLOD。必须比MinLOD大
		} 	D3D11_SAMPLER_DESC;

		D3D11_FILTER部分枚举含义如下：
		枚举值											缩小		放大		mipmap
		D3D11_FILTER_MIN_MAG_MIP_POINT					点采样		点采样		点采样
		D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR			点采样		点采样		线性采样
		D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT		点采样		线性采样	点采样
		D3D11_FILTER_MIN_MAG_MIP_LINEAR					线性采样	线性采样	线性采样
		D3D11_FILTER_ANISOTROPIC						各向异性	各向异性	各向异性

		D3D11_TEXTURE_ADDRESS_MODE是单个方向的寻址模式，
		有时候纹理坐标会超过1.0或者小于0.0，这时候寻址模式可以解释边界外的情况，
		含义如下：
			D3D11_TEXTURE_ADDRESS_WRAP是将指定纹理坐标分量的值[t, t + 1], t ∈ Z映射到[0.0, 1.0]，
			因此作用到u和v分量时看起来就像是把用一张贴图紧密平铺到其他位置上
			https://github.com/Life4gal/FromZero2D3D/blob/master/PIC/D3D11_TEXTURE_ADDRESS_WRAP.png

			D3D11_TEXTURE_ADDRESS_MIRROR在每个整数点处翻转纹理坐标值。例如u在[0.0, 1.0]按正常纹理坐标寻址，
			在[1.0, 2.0]内则翻转，在[2.0, 3.0]内又回到正常的寻址，以此类推
			https://github.com/Life4gal/FromZero2D3D/blob/master/PIC/D3D11_TEXTURE_ADDRESS_MIRROR.png

			D3D11_TEXTURE_ADDRESS_CLAMP对指定纹理坐标分量，小于0.0的值都取作0.0，
			大于1.0的值都取作1.0，在[0.0, 1.0]的纹理坐标不变
			https://github.com/Life4gal/FromZero2D3D/blob/master/PIC/D3D11_TEXTURE_ADDRESS_CLAMP.png

			D3D11_TEXTURE_ADDRESS_BORDER对于指定纹理坐标分量的值在[0.0, 1.0]外的区域都使用BorderColor进行填充
			https://github.com/Life4gal/FromZero2D3D/blob/master/PIC/D3D11_TEXTURE_ADDRESS_BORDER.png

			D3D11_TEXTURE_ADDRESS_MIRROR_ONCE相当于MIRROR和CLAMP的结合，仅[-1.0,1.0]的范围内镜像有效，
			若小于-1.0则取-1.0，大于1.0则取1.0，在[-1.0, 0.0]进行翻转
			https://github.com/Life4gal/FromZero2D3D/blob/master/PIC/D3D11_TEXTURE_ADDRESS_MIRROR_ONCE.png	
	*/
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	/*
		在C++代码层中，我们只能通过D3D设备创建采样器状态，然后绑定到渲染管线中，
		使得在HLSL中可以根据过滤器、寻址模式等进行采样。

		HRESULT ID3D11Device::CreateSamplerState(
			const D3D11_SAMPLER_DESC *pSamplerDesc, // [In]采样器状态描述
			ID3D11SamplerState **ppSamplerState);   // [Out]输出的采样器
	 */
	HR(m_pd3dDevice->CreateSamplerState(&sampDesc, m_pSamplerState.GetAddressOf()));

	// 初始化常量缓冲区的值
	// 初始化每帧可能会变化的值
	/*
		现在需要利用mBuffer结构体变量用于更新常量缓冲区，
		其中view和proj矩阵需要预先进行一次转置以抵消HLSL列主矩阵的转置，
		至于world矩阵已经是单位矩阵就不需要了
	 */
	m_CameraMode = CameraMode::FirstPerson;
	auto camera = std::make_shared<FirstPersonCamera>();
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, static_cast<float>(m_ClientWidth), static_cast<float>(m_ClientHeight));
	camera->LookAt(XMFLOAT3(), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	 // 初始化用于VS的常量缓冲区的值
	//m_VSConstantBuffer.world = XMMatrixIdentity();		// 单位矩阵的转置是它本身
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
	//m_VSConstantBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
	//	XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
	//	XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
	//	XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	//));
	/*
		// 透视投影矩阵
		XMMATRIX XMMatrixPerspectiveFovLH( // 返回投影矩阵
		    FLOAT FovAngleY,                   // 中心垂直弧度
		    FLOAT AspectRatio,                 // 宽高比
		    FLOAT NearZ,                       // 近平面距离
		    FLOAT FarZ);                       // 远平面距离
	 */
	//m_VSConstantBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));
	//m_VSConstantBuffer.worldInvTranspose = XMMatrixIdentity();

	// 初始化仅在窗口大小变动时修改的值
	m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
	m_CBOnResize.proj = XMMatrixTranspose(m_pCamera->GetProjXM());
	
	// 初始化不会变化的值
	// 环境光
	m_CBRarely.dirLight[0].ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.dirLight[0].diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_CBRarely.dirLight[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.dirLight[0].direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	// 灯光
	m_CBRarely.pointLight[0].position = XMFLOAT3(0.0f, 10.0f, 0.0f);
	m_CBRarely.pointLight[0].ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.pointLight[0].diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_CBRarely.pointLight[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.pointLight[0].att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_CBRarely.pointLight[0].range = 25.0f;
	m_CBRarely.numDirLight = 1;
	m_CBRarely.numPointLight = 1;
	m_CBRarely.numSpotLight = 0;
	// 初始化材质
	m_CBRarely.material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.material.diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	m_CBRarely.material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 50.0f);

	// 更新不容易被修改的常量缓冲区资源
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBChangesOnResize), &m_CBOnResize, sizeof(CBChangesOnResize));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[2].Get(), 0);

	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[3].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBChangesRarely), &m_CBRarely, sizeof(CBChangesRarely));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[3].Get(), 0);

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
	//D3D11_RASTERIZER_DESC rasterizerDesc;
	//ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	//rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	//rasterizerDesc.CullMode = D3D11_CULL_NONE;
	//rasterizerDesc.FrontCounterClockwise = false;
	//rasterizerDesc.DepthClipEnable = true;
	/*
		HRESULT ID3D11Device::CreateRasterizerState( 
			const D3D11_RASTERIZER_DESC *pRasterizerDesc,    // [In]光栅化状态描述
			ID3D11RasterizerState **ppRasterizerState) = 0;  // [Out]输出光栅化状态
	 */
	//HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireframe.GetAddressOf()));

	// ******************
	// 给渲染管线各个阶段绑定好所需资源
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
	m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout3D.Get());
	// 默认绑定3D着色器
	m_pd3dImmediateContext->VSSetShader(m_pVertexShader3D.Get(), nullptr, 0);

	// 预先绑定各自所需的缓冲区，其中每帧更新的缓冲区需要绑定到两个缓冲区上
	m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
	m_pd3dImmediateContext->VSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	m_pd3dImmediateContext->VSSetConstantBuffers(2, 1, m_pConstantBuffers[2].GetAddressOf());

	m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	m_pd3dImmediateContext->PSSetConstantBuffers(3, 1, m_pConstantBuffers[3].GetAddressOf());
	m_pd3dImmediateContext->PSSetShader(m_pPixelShader3D.Get(), nullptr, 0);
	m_pd3dImmediateContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

	/*
		void ID3D11DeviceContext::PSSetShaderResources(
			UINT StartSlot,	// [In]起始槽索引，对应HLSL的register(t*)
			UINT NumViews,	// [In]着色器资源视图数目
			ID3D11ShaderResourceView * const *ppShaderResourceViews	// [In]着色器资源视图数组
		);

		StartSlot填0,这样在HLSL里对应regisgter(t0)的g_Tex存放的就是木箱表面的纹理了
	 */
	//m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pWoodCrate.GetAddressOf());
	
	// ******************
	// 设置调试对象名
	//
	D3D11SetDebugObjectName(m_pVertexLayout2D.Get(), "VertexPosTexLayout");
	D3D11SetDebugObjectName(m_pVertexLayout3D.Get(), "VertexPosNormalTexLayout");
	D3D11SetDebugObjectName(m_pConstantBuffers[0].Get(), "CBDrawing");
	D3D11SetDebugObjectName(m_pConstantBuffers[1].Get(), "CBFrame");
	D3D11SetDebugObjectName(m_pConstantBuffers[2].Get(), "CBOnResize");
	D3D11SetDebugObjectName(m_pConstantBuffers[3].Get(), "CBRarely");
	D3D11SetDebugObjectName(m_pVertexShader2D.Get(), "Basic_VS_2D");
	D3D11SetDebugObjectName(m_pVertexShader3D.Get(), "Basic_VS_3D");
	D3D11SetDebugObjectName(m_pPixelShader2D.Get(), "Basic_PS_2D");
	D3D11SetDebugObjectName(m_pPixelShader3D.Get(), "Basic_PS_3D");
	D3D11SetDebugObjectName(m_pSamplerState.Get(), "SSLinearWrap");
	m_Floor.SetDebugObjectName("Floor");
	m_WoodCrate.SetDebugObjectName("WoodCrate");
	m_Walls[0].SetDebugObjectName("Walls[0]");
	m_Walls[1].SetDebugObjectName("Walls[1]");
	m_Walls[2].SetDebugObjectName("Walls[2]");
	m_Walls[3].SetDebugObjectName("Walls[3]");
	
	return true;
}

GameApp::GameObject::GameObject()
	:
	m_VertexStride(),
	m_IndexCount()
{
}

Transform& GameApp::GameObject::GetTransform()
{
	return m_Transform;
}

const Transform& GameApp::GameObject::GetTransform() const
{
	return m_Transform;
}

template<class VertexType, class IndexType>
void GameApp::GameObject::SetBuffer(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData)
{
	// 释放旧资源
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	// 设置顶点缓冲区描述
	m_VertexStride = sizeof(VertexType);
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = static_cast<UINT>(meshData.vertexVec.size()) * m_VertexStride;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	HR(device->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

	// 设置索引缓冲区描述
	m_IndexCount = static_cast<UINT>(meshData.indexVec.size());
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = m_IndexCount * sizeof(IndexType);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = meshData.indexVec.data();
	HR(device->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
}

void GameApp::GameObject::SetTexture(ID3D11ShaderResourceView* texture)
{
	m_pTexture = texture;
}

void GameApp::GameObject::Draw(ID3D11DeviceContext* deviceContext)
{
	// 设置顶点/索引缓冲区
	UINT strides = m_VertexStride;
	UINT offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);
	deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// 获取之前已经绑定到渲染管线上的常量缓冲区并进行修改
	ComPtr<ID3D11Buffer> cBuffer = nullptr;
	/*
		void ID3D11DeviceContext::VSGetConstantBuffers( 
		    UINT StartSlot,     // [In]指定的起始槽索引
		    UINT NumBuffers,    // [In]常量缓冲区数目 
		    ID3D11Buffer **ppConstantBuffers) = 0;    // [Out]常量固定缓冲区数组
	 */
	deviceContext->VSGetConstantBuffers(0, 1, cBuffer.GetAddressOf());

	// 内部进行转置
	const XMMATRIX W = m_Transform.GetLocalToWorldMatrixXM();
	CBChangesEveryDrawing cbDrawing
	{
		XMMatrixTranspose(W),
		XMMatrixInverse(nullptr, W)	// 两次转置抵消
	};
	
	// 更新常量缓冲区
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBChangesEveryDrawing), &cbDrawing, sizeof(CBChangesEveryDrawing));
	deviceContext->Unmap(cBuffer.Get(), 0);

	// 设置纹理
	deviceContext->PSSetShaderResources(0, 1, m_pTexture.GetAddressOf());
	// 可以开始绘制
	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}

void GameApp::GameObject::SetDebugObjectName(const std::string& name) const
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), name + ".VertexBuffer");
	D3D11SetDebugObjectName(m_pIndexBuffer.Get(), name + ".IndexBuffer");
#else
	UNREFERENCED_PARAMETER(name);
#endif
}
