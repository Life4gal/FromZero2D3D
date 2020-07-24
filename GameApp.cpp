#include "GameApp.h"

using namespace DirectX;

// ReSharper disable once CppParameterMayBeConst,不要给HINSTANCE附加顶层const声明,不然实际上会变成底层const
GameApp::GameApp(HINSTANCE hInstance)
	:
	D3DApp(hInstance),
	m_sphereRad(),
	m_cameraMode(CameraMode::ThirdPerson)
{
}

GameApp::~GameApp() = default;

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	// 务必先初始化所有渲染状态，以供下面的特效使用
	RenderStates::InitAll(m_pd3dDevice.Get());

	if (!m_basicEffect.InitAll(m_pd3dDevice.Get()))
		return false;

	if (!m_skyEffect.InitAll(m_pd3dDevice.Get()))
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
		m_pCamera->SetViewPort(0.0f, 0.0f, static_cast<float>(m_clientWidth), static_cast<float>(m_clientHeight));
		m_basicEffect.SetProjMatrix(m_pCamera->GetProjMatrix());
	}
}

// IMGUI是否需要获取鼠标控制权
bool g_isImguiCaptureMouse = false;

void GameApp::UpdateScene(const float dt)
{
	const Mouse::State mouseState = m_pMouse->GetState();
	m_mouseTracker.Update(mouseState);
	
	const Keyboard::State keyState = m_pKeyboard->GetState();
	m_keyboardTracker.Update(keyState);

	if (m_cameraMode == CameraMode::FirstPerson || (m_cameraMode == CameraMode::ThirdPerson && keyState.IsKeyDown(Keyboard::LeftControl)))
	{
		if (keyState.IsKeyDown(Keyboard::W))
		{
			m_player.Walk(dt * 6.0f);
		}
		if (keyState.IsKeyDown(Keyboard::S))
		{
			m_player.Walk(dt * -6.0f);
		}
		if (keyState.IsKeyDown(Keyboard::A))
		{
			m_player.Strafe(dt * -6.0f);
		}
		if (keyState.IsKeyDown(Keyboard::D))
		{
			m_player.Strafe(dt * 6.0f);
		}
		if (keyState.IsKeyDown(Keyboard::Q))
		{
			m_player.Turn(dt * -6.0f);
		}
		if (keyState.IsKeyDown(Keyboard::E))
		{
			m_player.Turn(dt * 6.0f);
		}
	}

	// 调整位置
	m_player.AdjustPosition();

	if (m_cameraMode == CameraMode::FirstPerson)
	{
		auto firstPersonCamera = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
		
		// 第一人称摄像机距物体中心偏一点
		const XMFLOAT3 position = m_player.GetPosition();
		firstPersonCamera->SetPosition(position.x, position.y + 1.5f, position.z + 1.5f);

		// 在鼠标没进入窗口前仍为ABSOLUTE模式
		if (mouseState.positionMode == Mouse::MODE_RELATIVE)
		{
			firstPersonCamera->Pitch(static_cast<float>(mouseState.y) * dt * 2.5f);
			firstPersonCamera->RotateY(static_cast<float>(mouseState.x) * dt * 2.5f);
		}

		if(keyState.IsKeyDown(Keyboard::LeftControl))
		{
			m_pMouse->SetMode(Mouse::MODE_ABSOLUTE);
			g_isImguiCaptureMouse = true;
		}
		else
		{
			m_pMouse->SetMode(Mouse::MODE_RELATIVE);
			g_isImguiCaptureMouse = false;
		}
	}
	else if(m_cameraMode == CameraMode::ThirdPerson)
	{
		g_isImguiCaptureMouse = false;
		
		auto thirdPersonCamera = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);
		
		// 设置目标
		thirdPersonCamera->SetTarget(m_player.GetPosition());
		// 绕物体旋转
		thirdPersonCamera->RotateX(static_cast<float>(mouseState.y) * dt * 2.5f);
		thirdPersonCamera->RotateY(static_cast<float>(mouseState.x) * dt * 2.5f);
		thirdPersonCamera->Approach(static_cast<float>(-mouseState.scrollWheelValue) / 120 * 1.0f);
	}
	
	// 更新观察矩阵
	m_basicEffect.SetViewMatrix(m_pCamera->GetViewMatrix());
	m_basicEffect.SetEyePos(m_pCamera->GetPositionVector());

	// 摄像机模式切换
	if (m_keyboardTracker.IsKeyPressed(Keyboard::D1) && m_cameraMode != CameraMode::FirstPerson)
	{
		// 先保存摄像机之前的方向,这样子切换视角不会导致摄像机方向变化
		const XMVECTOR look = m_pCamera->GetForwardAxisVector();
		const XMVECTOR up = m_pCamera->GetUpAxisVector();
		auto firstPersonCamera = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
		if (!firstPersonCamera)
		{
			firstPersonCamera.reset(new FirstPersonCamera);
			firstPersonCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = firstPersonCamera;
		}

		const XMFLOAT3 position = m_player.GetPosition();
		firstPersonCamera->LookTo(
			XMLoadFloat3(&position),
			look,
			up
		);

		m_cameraMode = CameraMode::FirstPerson;
	}
	else if (m_keyboardTracker.IsKeyPressed(Keyboard::D2) && m_cameraMode != CameraMode::ThirdPerson)
	{
		// 先保存摄像机之前的方向,这样子切换视角不会导致摄像机方向变化
		const XMVECTOR look = m_pCamera->GetForwardAxisVector();
		const XMVECTOR up = m_pCamera->GetUpAxisVector();
		auto thirdPersonCamera = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);
		if (!thirdPersonCamera)
		{
			thirdPersonCamera.reset(new ThirdPersonCamera);
			thirdPersonCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = thirdPersonCamera;
		}
		
		thirdPersonCamera->SetTarget(m_player.GetPosition());
		thirdPersonCamera->LookTo(look, up);
		thirdPersonCamera->SetDistance(8.0f);
		thirdPersonCamera->SetDistanceMinMax(3.0f, 20.0f);

		m_cameraMode = CameraMode::ThirdPerson;
	}

	// 设置球体动画速度
	m_sphereRad += 2.0f * dt;

	// 重置滚轮值
	m_pMouse->ResetScrollWheelValue();
	
	// 退出程序，这里应向窗口发送销毁信息
	if (keyState.IsKeyDown(Keyboard::Escape))
	{
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
	}

	m_imguiPanel.LoadData(m_player);
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	m_imguiPanel.Draw();

	// ******************
	// 绘制Direct3D部分
	//

	// ******************
	// 生成动态天空盒
	//
	
	// 保留当前绘制的渲染目标视图和深度模板视图
	m_pDaylight->Cache(m_pd3dImmediateContext.Get(), m_basicEffect);

	// 绘制动态天空盒的每个面（以球体为中心）
	for (int i = 0; i < 6; ++i)
	{
		m_pDaylight->BeginCapture(
			m_pd3dImmediateContext.Get(), m_basicEffect, XMFLOAT3(), static_cast<D3D11_TEXTURECUBE_FACE>(i));

		// 不绘制中心球
		DrawSceneObject(false);
	}

	// 恢复之前的绘制设定
	m_pDaylight->Restore(m_pd3dImmediateContext.Get(), m_basicEffect, *m_pCamera);
	
	// ******************
	// 绘制场景
	//

	// 预先清空
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), Colors::Black);
	/*
		void ID3D11DeviceContext::ClearDepthStencilView(
		    ID3D11DepthStencilView *pDepthStencilView,  // [In]深度模板视图
		    UINT ClearFlags,     // [In]使用D3D11_CLEAR_FLAG枚举类型决定需要清空的部分
		    FLOAT Depth,         // [In]使用Depth值填充所有元素的深度部分
		    UINT8 Stencil);      // [In]使用Stencil值填充所有元素的模板部分

		枚举值				含义
		D3D11_CLEAR_DEPTH	清空深度部分
		D3D11_CLEAR_STENCIL	清空模板部分

		通常深度值会默认设为1.0以确保任何在摄像机视野范围内的物体都能被显示出来
		模板值则默认会设置为0
	 */
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 绘制中心球
	DrawSceneObject(true);

	// 绘制Direct2D部分
	//
	if (m_pd2dRenderTarget != nullptr)
	{
		m_pd2dRenderTarget->BeginDraw();
		const std::wstring text =
			L"法线贴图\n";

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
	
	// 绘制Dear ImGui
	ImguiPanel::Present();
	
	HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitResource()
{
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\bricks_nmap.dds", nullptr, m_bricksNormalMap.GetAddressOf()));
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\stones_nmap.dds", nullptr, m_stonesNormalMap.GetAddressOf()));
	
	// ******************
	// 初始化天空盒相关

	m_pDaylight = std::make_unique<DynamicSkyRender>();
	HR(m_pDaylight->InitResource(
		m_pd3dDevice.Get(), 
		m_pd3dImmediateContext.Get(),
		L"Texture\\daylight.jpg",
		5000.0f, 256)
	);

	m_basicEffect.SetTextureCube(m_pDaylight->GetDynamicTextureCube());

	// ******************
	// 游戏对象

	// 玩家
	m_player.Init(m_pd3dDevice.Get());

	 // 地面
	{
		Model ground(m_pd3dDevice.Get(), Geometry::CreatePlane<VertexPosNormalTangentTex>(XMFLOAT2(16.0f, 16.0f), XMFLOAT2(8.0f, 8.0f)));
		ModelPart& modelPart = ground.modelParts.front();
		modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
		modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
		modelPart.material.reflect = XMFLOAT4();

		HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(),
			L"Texture\\stones.dds",
			nullptr,
			modelPart.texDiffuse.GetAddressOf())
		);
		
		m_ground.SetModel(std::move(ground));
		m_ground.GetTransform().SetPosition(0.0f, -1.0f, 0.0f);
	}
	// 球体
	{
		Model sphere(m_pd3dDevice.Get(), Geometry::CreateSphere(3.0f, 30, 30));
		ModelPart& modelPart = sphere.modelParts.front();
		modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		modelPart.material.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		modelPart.material.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
		modelPart.material.reflect = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);

		HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(),
			L"Texture\\bricks.dds",
			nullptr,
			modelPart.texDiffuse.GetAddressOf())
		);
		
		m_sphere.SetModel(std::move(sphere));
		m_sphere.ResizeBuffer(m_pd3dDevice.Get(), 5);
	}
	// 柱体
	{
		Model cylinder(m_pd3dDevice.Get(), Geometry::CreateCylinder<VertexPosNormalTangentTex>(0.75f, 3.0f));
		ModelPart& modelPart = cylinder.modelParts.front();
		modelPart.material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		modelPart.material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
		modelPart.material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
		modelPart.material.reflect = XMFLOAT4();
		
		HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(),
			L"Texture\\bricks.dds",
			nullptr,
			modelPart.texDiffuse.GetAddressOf())
		);
		
		m_cylinder.SetModel(std::move(cylinder));
		m_cylinder.ResizeBuffer(m_pd3dDevice.Get(), 5);
	}

	// ******************
	// 初始化摄像机
	//
	
	auto camera = std::make_shared<ThirdPersonCamera>();
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, static_cast<float>(m_clientWidth), static_cast<float>(m_clientHeight));
	camera->SetTarget(XMFLOAT3(0.0f, 0.5f, 0.0f));
	camera->SetDistance(8.0f);
	camera->SetDistanceMinMax(3.0f, 20.0f);
	camera->SetRotationX(XM_PIDIV4);
	camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);

	m_basicEffect.SetWorldMatrix(XMMatrixIdentity());
	m_basicEffect.SetViewMatrix(camera->GetViewMatrix());
	m_basicEffect.SetProjMatrix(camera->GetProjMatrix());
	m_basicEffect.SetEyePos(camera->GetPositionVector());
	
	// ******************
	// 初始化不会变化的值
	//

	// 方向光
	DirectionalLight dirLight[4];
	dirLight[0].ambient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
	dirLight[0].diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight[0].specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	dirLight[0].direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
	dirLight[1] = dirLight[0];
	dirLight[1].direction = XMFLOAT3(0.577f, -0.577f, 0.577f);
	dirLight[2] = dirLight[0];
	dirLight[2].direction = XMFLOAT3(0.577f, -0.577f, -0.577f);
	dirLight[3] = dirLight[0];
	dirLight[3].direction = XMFLOAT3(-0.577f, -0.577f, -0.577f);
	for (int i = 0; i < 4; ++i)
	{
		m_basicEffect.SetDirLight(i, dirLight[i]);
	}

	// ******************
	// 设置调试对象名
	//
	m_ground.SetDebugObjectName("Ground");
	m_cylinder.SetDebugObjectName("Cylinder");
	
	m_sphere.SetDebugObjectName("Sphere");

	m_pDaylight->SetDebugObjectName("DayLight");
	
	return true;
}

void GameApp::DrawSceneObject(const bool drawCenterSphere)
{
	// 绘制模型
	
	// 绘制玩家
	{
		m_basicEffect.SetRenderDefault(m_pd3dImmediateContext.Get(), BasicEffect::RenderType::RenderObject);
		m_basicEffect.SetReflectionEnabled(true);
		m_basicEffect.SetTextureUsed(true);
		m_player.Draw(m_pd3dImmediateContext.Get(), m_basicEffect);
	}

	// 只绘制球体的反射效果
	if (drawCenterSphere)
	{
		m_basicEffect.SetReflectionEnabled(true);
		m_basicEffect.SetRefractionEnabled(false);
		m_sphere.GetTransform().SetPosition(0.0f, 4.01f, 0.f);
		m_sphere.Draw(m_pd3dImmediateContext.Get(), m_basicEffect);
	}
	// 其他物体不反射
	m_basicEffect.SetReflectionEnabled(false);
	m_basicEffect.SetRefractionEnabled(false);
	// 绘制地面
	{
		m_basicEffect.SetRenderWithNormalMap(m_pd3dImmediateContext.Get(), BasicEffect::RenderType::RenderObject);
		m_basicEffect.SetTextureNormalMap(m_stonesNormalMap.Get());
		m_ground.Draw(m_pd3dImmediateContext.Get(), m_basicEffect);
	}

	// 绘制五个圆柱
	// 需要固定位置
	static std::vector<BasicTransform> cylinderWorlds = {
		BasicTransform(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(0.0f, 0.51f, 0.0f)),
		BasicTransform(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(7.5f, 0.51f, 7.5f)),
		BasicTransform(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(-7.5f, 0.51f, 7.5f)),
		BasicTransform(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(-7.5f, 0.51f, -7.5f)),
		BasicTransform(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(7.5f, 0.51f, -7.5f))
	};
	{
		m_basicEffect.SetRenderWithNormalMap(m_pd3dImmediateContext.Get(), BasicEffect::RenderType::RenderInstance);
		m_basicEffect.SetTextureNormalMap(m_bricksNormalMap.Get());
		m_cylinder.DrawInstanced(m_pd3dImmediateContext.Get(), m_basicEffect, cylinderWorlds);
	}

	// 绘制五个圆球
	const std::vector<BasicTransform> sphereWorlds = {
		BasicTransform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(7.5f, 4.01f + 0.5f * XMScalarSin(m_sphereRad), 7.5f)),
		BasicTransform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(-7.5f, 4.01f + 0.5f * XMScalarSin(m_sphereRad), 7.5f)),
		BasicTransform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(-7.5f, 4.01f + 0.5f * XMScalarSin(m_sphereRad), -7.5f)),
		BasicTransform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(7.5f, 4.01f + 0.5f * XMScalarSin(m_sphereRad), -7.5f)),
		BasicTransform(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(4.51f * XMScalarCos(m_sphereRad), 4.01f + 1.5f * XMScalarSin(m_sphereRad), 4.51f * XMScalarSin(m_sphereRad)))
	};
	{
		m_basicEffect.SetRenderDefault(m_pd3dImmediateContext.Get(), BasicEffect::RenderType::RenderInstance);
		m_sphere.DrawInstanced(m_pd3dImmediateContext.Get(), m_basicEffect, sphereWorlds);
	}

	// 绘制天空盒
	{
		m_skyEffect.SetRenderDefault(m_pd3dImmediateContext.Get());
		
		if (drawCenterSphere)
			m_pDaylight->Draw(m_pd3dImmediateContext.Get(), m_skyEffect, *m_pCamera);
		else
			m_pDaylight->Draw(m_pd3dImmediateContext.Get(), m_skyEffect, m_pDaylight->GetCamera());
	}
}
