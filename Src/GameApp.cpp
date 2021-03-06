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
	m_cameraMode(CameraMode::THIRD_PERSON)
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
	m_pMouse->SetMode(Mouse::Mode::MODE_RELATIVE);

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

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

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
		HR(m_pd2dRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::White),
			m_pColorBrush.GetAddressOf())
		);
		
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

	// 只有处于相对模式我们才允许操作
	if (mouseState.positionMode == Mouse::Mode::MODE_RELATIVE)
	{
		// 第一人称或者第三人称按住左CTRL键才能移动玩家
		// 操作玩家需要长按操作键,所以我们使用 keyState.IsKeyDown() 而不是 m_keyboardTracker.IsKeyPressed()
		if (m_cameraMode == CameraMode::FIRST_PERSON || m_cameraMode == CameraMode::THIRD_PERSON && keyState.IsKeyDown(Keyboard::Keys::LEFT_CONTROL))
		{
			if (keyState.IsKeyDown(Keyboard::Keys::W))
			{
				m_player.Walk(dt * 6.0f);
			}
			if (keyState.IsKeyDown(Keyboard::Keys::S))
			{
				m_player.Walk(dt * -6.0f);
			}
			if (keyState.IsKeyDown(Keyboard::Keys::A))
			{
				m_player.Strafe(dt * -6.0f);
			}
			if (keyState.IsKeyDown(Keyboard::Keys::D))
			{
				m_player.Strafe(dt * 6.0f);
			}
			if (keyState.IsKeyDown(Keyboard::Keys::Q))
			{
				m_player.Turn(dt * -6.0f);
			}
			if (keyState.IsKeyDown(Keyboard::Keys::E))
			{
				m_player.Turn(dt * 6.0f);
			}
		}
		else if (m_cameraMode == CameraMode::FREE)
		{
			auto firstPersonCamera = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);

			if (keyState.IsKeyDown(Keyboard::Keys::W))
			{
				firstPersonCamera->MoveForward(dt * 6.0f);
			}
			if (keyState.IsKeyDown(Keyboard::Keys::S))
			{
				firstPersonCamera->MoveForward(dt * -6.0f);
			}
			if (keyState.IsKeyDown(Keyboard::Keys::A))
			{
				firstPersonCamera->Strafe(dt * -6.0f);
			}
			if (keyState.IsKeyDown(Keyboard::Keys::D))
			{
				firstPersonCamera->Strafe(dt * 6.0f);
			}

			firstPersonCamera->Pitch(static_cast<float>(mouseState.y) * dt * 2.5f);
			firstPersonCamera->RotateY(static_cast<float>(mouseState.x) * dt * 2.5f);

			// 只有在自由模式下我们允许鼠标切换为绝对模式以使用IMGUI
			if(m_keyboardTracker.IsKeyPressed(Keyboard::Keys::LEFT_CONTROL))
			{
				m_pMouse->SetMode(Mouse::Mode::MODE_ABSOLUTE);
				ImguiPanel::SetPanelCanUseKBandMouse(true);
			}
		}

		// 调整位置
		// 只有相对模式可以移动玩家,那么我们也只在相对模式调整玩家位置
		m_player.AdjustPosition({ { -50.0f, 0.5f, -50.0f, 0.0f } }, { { 50.0f, 0.5f , 50.0f, 0.0f } });

		if (m_cameraMode == CameraMode::FIRST_PERSON)
		{
			auto firstPersonCamera = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);

			// 第一人称摄像机距物体中心偏一点
			const XMFLOAT3 position = m_player.GetPosition();
			firstPersonCamera->SetPosition(position.x, position.y + 3.5f, position.z);
			firstPersonCamera->Pitch(static_cast<float>(mouseState.y) * dt * 2.5f);
			firstPersonCamera->RotateY(static_cast<float>(mouseState.x) * dt * 2.5f);
		}
		else if (m_cameraMode == CameraMode::THIRD_PERSON)
		{
			auto thirdPersonCamera = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);

			// 设置目标
			thirdPersonCamera->SetTarget(m_player.GetPosition());
			thirdPersonCamera->RotateX(static_cast<float>(mouseState.y) * dt * 2.5f);
			thirdPersonCamera->RotateY(static_cast<float>(mouseState.x) * dt * 2.5f);
			// 与目标物体的距离
			thirdPersonCamera->Approach(static_cast<float>(-mouseState.scrollWheelValue) / 120 * 1.0f);
		}

		// 摄像机模式切换
		if (m_keyboardTracker.IsKeyPressed(Keyboard::Keys::D8))
		{
			// 从第三人称或者从自由视角切换过来才要变
			if (m_cameraMode == CameraMode::THIRD_PERSON || m_cameraMode == CameraMode::FREE)
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
			}

			if (m_cameraMode == CameraMode::THIRD_PERSON)
			{
				m_cameraMode = CameraMode::FIRST_PERSON;
			}
			else
			{
				m_cameraMode = m_cameraMode != CameraMode::FIRST_PERSON ? CameraMode::FIRST_PERSON : CameraMode::FREE;
			}
		}
		else if (m_keyboardTracker.IsKeyPressed(Keyboard::Keys::D9) && m_cameraMode != CameraMode::THIRD_PERSON)
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

			m_cameraMode = CameraMode::THIRD_PERSON;
		}

		// 更新观察矩阵
		// 只有在相对模式下我们才可能会变动摄像机,所以我们只在相对模式下更新
		m_pBasicEffect->SetViewMatrix(m_pCamera->GetViewMatrix());
		m_pBasicEffect->SetEyePos(m_pCamera->GetPositionVector());
	}
	else if(mouseState.positionMode == Mouse::Mode::MODE_ABSOLUTE)
	{
		// 只有在自由视角才能切换为绝对模式
		// 在绝对模式下按左CTRL键切换为相对模式
		if (m_keyboardTracker.IsKeyPressed(Keyboard::Keys::LEFT_CONTROL))
		{
			m_pMouse->SetMode(Mouse::Mode::MODE_RELATIVE);
			ImguiPanel::SetPanelCanUseKBandMouse(false);
		}
		// TODO 鼠标在窗口之外也是绝对模式,我们需要另外的判断
		// 暂时我们只能先设置为切换为自由模式
		if(m_cameraMode != CameraMode::FREE)
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

			m_cameraMode = CameraMode::FREE;
		}
	}

	// 调整光线倾斜
	// 当我们增加光线的倾斜程度时，阴影粉刺会出现得愈发严重
	switch (m_slopeIndex)
	{
	case 0:
		m_originalLightDirs[0] = XMFLOAT3(1.0f / sqrtf(2.0f), -1.0f / sqrtf(2.0f), 0.0f);
		break;
	case 1:
		m_originalLightDirs[0] = XMFLOAT3(3.0f / sqrtf(13.0f), -2.0f / sqrtf(13.0f), 0.0f);
		break;
	case 2:
		m_originalLightDirs[0] = XMFLOAT3(2.0f / sqrtf(5.0f), -1.0f / sqrtf(5.0f), 0.0f);
		break;
	case 3:
		m_originalLightDirs[0] = XMFLOAT3(3.0f / sqrtf(10.0f), -1.0f / sqrtf(10.0f), 0.0f);
		break;
	default:
		m_originalLightDirs[0] = XMFLOAT3(4.0f / sqrtf(17.0f), -1.0f / sqrtf(17.0f), 0.0f);
	}
	
	// 更新光照
	static float theta = 0;
	theta += dt * XM_2PI / 40.0f;

	for (int i = 0; i < 3; ++i)
	{
		XMStoreFloat3(&m_dirLights[i].direction, XMVector3Transform(XMLoadFloat3(&m_originalLightDirs[i]), XMMatrixRotationY(theta)));
		
		m_pBasicEffect->SetDirLight(i, m_dirLights[i]);
	}

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
	m_pBasicEffect->SetShadowTransformMatrix(lightView * XMMatrixOrthographicLH(100.0f, 100.0f, 0.0f, 100.0f) * transform);

	// 重置滚轮值
	m_pMouse->ResetScrollWheelValue();
	
	// 退出程序，这里应向窗口发送销毁信息
	if (m_keyboardTracker.IsKeyPressed(Keyboard::Keys::ESCAPE))
	{
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
	}
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	ImguiPanel::Draw();

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
		m_pd2dRenderTarget->BeginDraw();
		std::wstring text =
			L"当前摄像机模式: ";
		switch (m_cameraMode)
		{
		case CameraMode::FIRST_PERSON:
			text += L"第一人称";
			break;
		case CameraMode::THIRD_PERSON:
			text += L"第三人称";
			break;
		default:
			{
				text += L"自由视角\n当前控制: ";
				if (m_pMouse->GetState().positionMode == Mouse::Mode::MODE_ABSOLUTE)
					text += L"IMGUI面板";
				else
					text += L"摄像机";
				text += L"\n(按左CTRL以切换对IMGUI和摄像机的控制)";
			}
		}
		text += L"\n(主键盘8在第一人称和自由视角间切换,主键盘9切换第三人称)\n";

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
	ImguiPanel::LoadData(&m_slopeIndex, &m_enableDebug, &m_grayMode);
	
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

	m_pShadowEffect->SetProjMatrix(XMMatrixOrthographicLH(100.0f, 100.0f, 0.0f, 100.0f));

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
	}
	// 柱子和球的位置
	{
		m_sphereTransforms.resize(90);
		m_cylinderTransforms.resize(90);

		// 前10个离得太近了,我们跳过
		for(int i = 0; i < 45; ++i)
		{
			const float j = static_cast<float>(i) + 5.0f;
			const float x = (5 + (50.f - j) * 0.4f) * (2 * sinf(XM_PI * j / 50) - sinf(XM_2PI * j / 50));
			const float z = 12 + 15 * (2 * cosf(XM_PI * j / 50) - cosf(XM_2PI * j / 50));

			m_sphereTransforms[i].SetPosition(x, 5.51f, z);
			m_sphereTransforms[i].SetScale(0.35f, 0.35f, 0.35f);
			m_sphereTransforms[static_cast<size_t>(89) - i].SetPosition(-x, 5.51f, z);
			m_sphereTransforms[static_cast<size_t>(89) - i].SetScale(0.35f, 0.35f, 0.35f);

			m_cylinderTransforms[i].SetPosition(x, 0.51f, z);
			m_cylinderTransforms[i].SetScale(0.35f, 1.0f, 0.35f);
			m_cylinderTransforms[static_cast<size_t>(89) - i].SetPosition(-x, 0.51f, z);
			m_cylinderTransforms[static_cast<size_t>(89) - i].SetScale(0.35f, 1.0f, 0.35f);
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
	m_pShadowMap->SetDebugObjectName("ShadowMap");
	m_pDaylight->SetDebugObjectName("DayLight");
	
	return true;
}
