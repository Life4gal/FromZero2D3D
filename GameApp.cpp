#include "GameApp.h"

using namespace DirectX;

// ReSharper disable once CppParameterMayBeConst,不要给HINSTANCE附加顶层const声明,不然实际上会变成底层const
GameApp::GameApp(HINSTANCE hInstance)
	:
	D3DApp(hInstance),
	m_enableDebug(true),
	m_grayMode(true),
	m_slopeIndex(),
	m_dirLights{},
	m_originalLightDirs{},
	m_pBasicEffect(std::make_unique<BasicEffect>()),
	m_pShadowEffect(std::make_unique<ShadowEffect>()),
	m_pSkyEffect(std::make_unique<SkyEffect>()),
	m_pDebugEffect(std::make_unique<DebugEffect>()),
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

	if (!m_pBasicEffect->InitAll(m_pd3dDevice.Get()))
		return false;

	if (!m_pSkyEffect->InitAll(m_pd3dDevice.Get()))
		return false;

	if (!m_pShadowEffect->InitAll(m_pd3dDevice.Get()))
		return false;

	if (!m_pDebugEffect->InitAll(m_pd3dDevice.Get()))
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
		m_pBasicEffect->SetProjMatrix(m_pCamera->GetProjMatrix());
	}
}

void GameApp::UpdateScene(const float dt)
{
	const Mouse::State mouseState = m_pMouse->GetState();
	m_mouseTracker.Update(mouseState);
	
	const Keyboard::State keyState = m_pKeyboard->GetState();
	m_keyboardTracker.Update(keyState);

	// 调试模式开关
	if (m_keyboardTracker.IsKeyPressed(Keyboard::E))
		m_enableDebug = !m_enableDebug;
	// 灰度模式开关
	if (m_keyboardTracker.IsKeyPressed(Keyboard::G))
		m_grayMode = !m_grayMode;
	
	// 调整光线倾斜
	// 当我们增加光线的倾斜程度时，阴影粉刺会出现得愈发严重
	if (m_keyboardTracker.IsKeyPressed(Keyboard::D1))
	{
		m_originalLightDirs[0] = XMFLOAT3(1.0f / sqrtf(2.0f), -1.0f / sqrtf(2.0f), 0.0f);
		m_slopeIndex = 0;
	}
	if (m_keyboardTracker.IsKeyPressed(Keyboard::D2))
	{
		m_originalLightDirs[0] = XMFLOAT3(3.0f / sqrtf(13.0f), -2.0f / sqrtf(13.0f), 0.0f);
		m_slopeIndex = 1;
	}
	if (m_keyboardTracker.IsKeyPressed(Keyboard::D3))
	{
		m_originalLightDirs[0] = XMFLOAT3(2.0f / sqrtf(5.0f), -1.0f / sqrtf(5.0f), 0.0f);
		m_slopeIndex = 2;
	}
	if (m_keyboardTracker.IsKeyPressed(Keyboard::D4))
	{
		m_originalLightDirs[0] = XMFLOAT3(3.0f / sqrtf(10.0f), -1.0f / sqrtf(10.0f), 0.0f);
		m_slopeIndex = 3;
	}
	if (m_keyboardTracker.IsKeyPressed(Keyboard::D5))
	{
		m_originalLightDirs[0] = XMFLOAT3(4.0f / sqrtf(17.0f), -1.0f / sqrtf(17.0f), 0.0f);
		m_slopeIndex = 4;
	}
	
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
	m_player.AdjustPosition({ { -50.0f, 0.5f, -50.0f, 0.0f } }, { { 50.0f, 0.5f , 50.0f, 0.0f } });

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
			// 我们不再需要一个flag来标识IMGUI是否需要鼠标了
			// 只有在绝对模式下我们才能操作IMGUI
			m_pMouse->SetMode(Mouse::MODE_ABSOLUTE);
		}
		else
		{
			m_pMouse->SetMode(Mouse::MODE_RELATIVE);
		}
	}
	else if(m_cameraMode == CameraMode::ThirdPerson)
	{
		auto thirdPersonCamera = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);
		
		// 设置目标
		thirdPersonCamera->SetTarget(m_player.GetPosition());
		// 绕物体旋转
		thirdPersonCamera->RotateX(static_cast<float>(mouseState.y) * dt * 2.5f);
		thirdPersonCamera->RotateY(static_cast<float>(mouseState.x) * dt * 2.5f);
		thirdPersonCamera->Approach(static_cast<float>(-mouseState.scrollWheelValue) / 120 * 1.0f);
	}
	
	// 更新观察矩阵
	m_pBasicEffect->SetViewMatrix(m_pCamera->GetViewMatrix());
	m_pBasicEffect->SetEyePos(m_pCamera->GetPositionVector());

	// 摄像机模式切换
	if (m_keyboardTracker.IsKeyPressed(Keyboard::D0) && m_cameraMode != CameraMode::FirstPerson)
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
	else if (m_keyboardTracker.IsKeyPressed(Keyboard::D9) && m_cameraMode != CameraMode::ThirdPerson)
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

	// 更新光照
	static float theta = 0;
	theta += dt * XM_2PI / 40.0f;

	for (int i = 0; i < 3; ++i)
	{
		XMVECTOR dirVec = XMLoadFloat3(&m_originalLightDirs[i]);
		dirVec = XMVector3Transform(dirVec, XMMatrixRotationY(theta));
		XMStoreFloat3(&m_dirLights[i].direction, dirVec);
		m_pBasicEffect->SetDirLight(i, m_dirLights[i]);
	}

	//
	// 投影区域为正方体，以原点为中心，以方向光为+Z朝向
	const XMMATRIX lightView = XMMatrixLookAtLH(XMLoadFloat3(&m_dirLights[0].direction) * 20.0f * -2.0f, g_XMZero, g_XMIdentityR1);
	m_pShadowEffect->SetViewMatrix(lightView);

	// 将NDC空间 [-1, +1]^2 变换到纹理坐标空间 [0, 1]^2
	static XMMATRIX transform
	(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	);
	// S = V * P * T
	m_pBasicEffect->SetShadowTransformMatrix(lightView * XMMatrixOrthographicLH(100.0f, 100.0f, 20.0f, 60.0f) * transform);

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

	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), Colors::Silver);
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	// ******************
	// 绘制到阴影贴图
	//
	m_pShadowMap->Begin(m_pd3dImmediateContext.Get(), nullptr);
	{
		DrawScene(m_pShadowEffect.get());
	}
	m_pShadowMap->End(m_pd3dImmediateContext.Get());

	// ******************
	// 正常绘制场景
	//
	m_pBasicEffect->SetTextureShadowMap(m_pShadowMap->GetOutputTexture());
	DrawScene(m_pBasicEffect.get());
	
	// 绘制天空盒
	m_pSkyEffect->SetRenderDefault(m_pd3dImmediateContext.Get());
	m_pDaylight->Draw(m_pd3dImmediateContext.Get(), *m_pSkyEffect, *m_pCamera);

	// 解除深度缓冲区绑定
	m_pBasicEffect->SetTextureShadowMap(nullptr);
	m_pBasicEffect->Apply(m_pd3dImmediateContext.Get());

	// ******************
	// 调试绘制阴影贴图
	//
	if (m_enableDebug)
	{
		if (m_grayMode)
		{
			m_pDebugEffect->SetRenderOneComponentGray(m_pd3dImmediateContext.Get(), 0);
		}
		else
		{
			m_pDebugEffect->SetRenderOneComponent(m_pd3dImmediateContext.Get(), 0);
		}

		m_debugQuad.Draw(m_pd3dImmediateContext.Get(), m_pDebugEffect.get());
		// 解除绑定
		m_pDebugEffect->SetTextureDiffuse(nullptr);
		m_pDebugEffect->Apply(m_pd3dImmediateContext.Get());
	}

	// 绘制Direct2D部分
	//
	if (m_pd2dRenderTarget != nullptr)
	{
		static const float Slopes[5] = { 1.0f, 1.5f, 2.0f, 3.0f, 4.0f };
		
		m_pd2dRenderTarget->BeginDraw();
		std::wstring text =
			L"调试深度图: " + (m_enableDebug ? std::wstring(L"开") : std::wstring(L"关")) + L" (E切换)\n";
		if (m_enableDebug)
			text += L"G-灰度/单通道色显示切换\n";
		text += L"方向光倾斜: " + std::to_wstring(Slopes[m_slopeIndex]) + L" (主键盘1-5切换)\n";

		m_pd2dRenderTarget->DrawTextW(text.c_str(), static_cast<UINT32>(text.length()), m_pTextFormat.Get(),
			D2D1_RECT_F{ 0.0f, 0.0f, 600.0f, 200.0f }, m_pColorBrush.Get());
		HR(m_pd2dRenderTarget->EndDraw());
	}
	
	// 绘制Dear ImGui
	ImguiPanel::Present();
	
	HR(m_pSwapChain->Present(0, 0));
}

void GameApp::DrawScene(BasicEffect* pBasicEffect)
{
	// 地面
	pBasicEffect->SetRenderWithNormalMap(m_pd3dImmediateContext.Get(), IEffect::RenderType::RenderObject);
	m_ground.Draw(m_pd3dImmediateContext.Get(), pBasicEffect);

	// 石柱
	pBasicEffect->SetRenderWithNormalMap(m_pd3dImmediateContext.Get(), IEffect::RenderType::RenderInstance);
	m_cylinder.DrawInstanced(m_pd3dImmediateContext.Get(), pBasicEffect, m_cylinderTransforms);

	// 石球
	pBasicEffect->SetRenderWithNormalMap(m_pd3dImmediateContext.Get(), IEffect::RenderType::RenderInstance);
	m_sphere.DrawInstanced(m_pd3dImmediateContext.Get(), pBasicEffect, m_sphereTransforms);
	
	// 玩家
	pBasicEffect->SetRenderDefault(m_pd3dImmediateContext.Get(), IEffect::RenderType::RenderObject);
	m_player.Draw(m_pd3dImmediateContext.Get(), pBasicEffect);
}

void GameApp::DrawScene(ShadowEffect* pShadowEffect)
{
	// 地面
	pShadowEffect->SetRenderDefault(m_pd3dImmediateContext.Get(), IEffect::RenderType::RenderObject);
	m_ground.Draw(m_pd3dImmediateContext.Get(), pShadowEffect);

	// 石柱
	pShadowEffect->SetRenderDefault(m_pd3dImmediateContext.Get(), IEffect::RenderType::RenderInstance);
	m_cylinder.DrawInstanced(m_pd3dImmediateContext.Get(), pShadowEffect, m_cylinderTransforms);

	// 石球
	pShadowEffect->SetRenderDefault(m_pd3dImmediateContext.Get(), IEffect::RenderType::RenderInstance);
	m_sphere.DrawInstanced(m_pd3dImmediateContext.Get(), pShadowEffect, m_sphereTransforms);

	// 玩家
	pShadowEffect->SetRenderDefault(m_pd3dImmediateContext.Get(), IEffect::RenderType::RenderObject);
	m_player.Draw(m_pd3dImmediateContext.Get(), pShadowEffect);
}

bool GameApp::InitResource()
{
	// ******************
	// 初始化天空盒相关

	m_pDaylight = std::make_unique<SkyRender>();
	HR(m_pDaylight->InitResource(
		m_pd3dDevice.Get(), 
		m_pd3dImmediateContext.Get(),
		L"Texture\\daylight.jpg",
		5000.0f)
	);

	m_pBasicEffect->SetTextureCube(m_pDaylight->GetTextureCube());

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

	// ******************
	// 初始化阴影贴图和特效
	m_pShadowMap = std::make_unique<TextureRender>();
	HR(m_pShadowMap->InitResource(m_pd3dDevice.Get(), 2048, 2048, true));

	// 开启纹理、阴影
	m_pBasicEffect->SetTextureUsed(true);
	m_pBasicEffect->SetShadowEnabled(true);
	m_pBasicEffect->SetViewMatrix(camera->GetViewMatrix());
	m_pBasicEffect->SetProjMatrix(camera->GetProjMatrix());

	m_pShadowEffect->SetProjMatrix(XMMatrixOrthographicLH(100.0f, 100.0f, 20.0f, 60.0f));

	m_pDebugEffect->SetWorldMatrix(XMMatrixIdentity());
	m_pDebugEffect->SetViewMatrix(XMMatrixIdentity());
	m_pDebugEffect->SetProjMatrix(XMMatrixIdentity());
	
	// ******************
	// 初始化对象

	// 玩家
	m_player.Init(m_pd3dDevice.Get());
	m_player.SetPosition({ 0.0f, 0.6f, -20.0f });

	 // 地面
	{
		Model ground(m_pd3dDevice.Get(), Geometry::CreatePlane<VertexPosNormalTangentTex>(XMFLOAT2(100.0f, 100.0f), XMFLOAT2(6.0f, 9.0f)));
		ModelPart& modelPart = ground.modelParts.front();
		modelPart.material.ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
		modelPart.material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		modelPart.material.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
		modelPart.material.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

		HR(CreateDDSTextureFromFile(
			m_pd3dDevice.Get(),
			L"Texture\\floor.dds",
			nullptr,
			modelPart.texDiffuse.GetAddressOf())
		);
		HR(CreateDDSTextureFromFile(
			m_pd3dDevice.Get(),
			L"Texture\\floor_nmap.dds",
			nullptr,
			modelPart.texNormalMap.GetAddressOf())
		);
		
		m_ground.SetModel(std::move(ground));
		m_ground.GetTransform().SetPosition(0.0f, -1.0f, 0.0f);
	}
	// 球体
	{
		Model sphere(m_pd3dDevice.Get(), Geometry::CreateSphere<VertexPosNormalTangentTex>(3.0f, 30, 30));
		ModelPart& modelPart = sphere.modelParts.front();
		modelPart.material.ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
		modelPart.material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		modelPart.material.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
		modelPart.material.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

		HR(CreateDDSTextureFromFile(
			m_pd3dDevice.Get(),
			L"Texture\\stone.dds",
			nullptr,
			modelPart.texDiffuse.GetAddressOf())
		);
		
		HR(CreateDDSTextureFromFile(
			m_pd3dDevice.Get(),
			L"Texture\\stone_nmap.dds",
			nullptr,
			modelPart.texNormalMap.GetAddressOf())
		);
		
		m_sphere.SetModel(std::move(sphere));

		m_sphereTransforms.resize(100);
		for (int i = 0; i < 100; ++i)
		{
			m_sphereTransforms[i].SetPosition(
				15 * (2 * cosf(XM_PI * static_cast<float>(i) / 50) - cosf(XM_2PI * static_cast<float>(i) / 50)), 
				5.51f, 
				15 * (2 * sinf(XM_PI * static_cast<float>(i) / 50) - sinf(XM_2PI * static_cast<float>(i) / 50))
			);
			m_sphereTransforms[i].SetScale(0.35f, 0.35f, 0.35f);
		}
	}
	// 柱体
	{
		Model cylinder(m_pd3dDevice.Get(), Geometry::CreateCylinder<VertexPosNormalTangentTex>(0.75f, 3.0f));
		ModelPart& modelPart = cylinder.modelParts.front();
		modelPart.material.ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		modelPart.material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		modelPart.material.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
		modelPart.material.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		
		HR(CreateDDSTextureFromFile(
			m_pd3dDevice.Get(),
			L"Texture\\bricks.dds",
			nullptr,
			modelPart.texDiffuse.GetAddressOf())
		);
		
		HR(CreateDDSTextureFromFile(
			m_pd3dDevice.Get(),
			L"Texture\\bricks_nmap.dds",
			nullptr,
			modelPart.texNormalMap.GetAddressOf())
		);
		
		m_cylinder.SetModel(std::move(cylinder));
		
		m_cylinderTransforms.resize(100);
		for (int i = 0; i < 100; ++i)
		{
			m_cylinderTransforms[i].SetPosition(
				15 * (2 * cosf(XM_PI * static_cast<float>(i) / 50) - cosf(XM_2PI * static_cast<float>(i) / 50)),
				0.51f,
				15 * (2 * sinf(XM_PI * static_cast<float>(i) / 50) - sinf(XM_2PI * static_cast<float>(i) / 50))
			);
			m_cylinderTransforms[i].SetScale(0.35f, 1.0f, 0.35f);
		}
	}

	// 调试用矩形
	Model quadModel;
	quadModel.SetMesh(m_pd3dDevice.Get(), Geometry::Create2DShow<VertexPosNormalTex>(XMFLOAT2(0.6f, -0.6f), XMFLOAT2(0.4f, 0.4f)));
	quadModel.modelParts.front().texDiffuse = m_pShadowMap->GetOutputTexture();
	m_debugQuad.SetModel(std::move(quadModel));
	
	// ******************
	// 初始化不会变化的值
	//

	m_dirLights[0].ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_dirLights[0].diffuse = XMFLOAT4(0.7f, 0.7f, 0.6f, 1.0f);
	m_dirLights[0].specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	m_dirLights[0].direction = XMFLOAT3(5.0f / sqrtf(50.0f), -5.0f / sqrtf(50.0f), 0.0f);

	m_dirLights[1].ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_dirLights[1].diffuse = XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
	m_dirLights[1].specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_dirLights[1].direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	m_dirLights[2].ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_dirLights[2].diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_dirLights[2].specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_dirLights[2].direction = XMFLOAT3(0.0f, 0.0, -1.0f);

	for (int i = 0; i < 3; ++i)
	{
		m_originalLightDirs[i] = m_dirLights[i].direction;
		m_pBasicEffect->SetDirLight(i, m_dirLights[i]);
	}

	// ******************
	// 设置调试对象名
	//
	
	m_ground.SetDebugObjectName("Ground");
	m_cylinder.SetDebugObjectName("Cylinder");
	m_sphere.SetDebugObjectName("Sphere");
	m_debugQuad.SetDebugObjectName("DebugQuad");
	// 作为阴影RTV是空的
	//m_pShadowMap->SetDebugObjectName("ShadowMap");
	m_pDaylight->SetDebugObjectName("DayLight");
	
	return true;
}
