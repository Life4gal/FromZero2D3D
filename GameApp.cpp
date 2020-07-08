#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance)
	:
	D3DApp(hInstance),
	m_ShadowMat(),
	m_WoodCrateMat(),
	m_CameraMode(CameraMode::ThirdPerson)
{
}

GameApp::~GameApp() = default;

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	// 务必先初始化所有渲染状态，以供下面的特效使用
	RenderStates::InitAll(m_pd3dDevice.Get());

	if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
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
		m_pCamera->SetViewPort(0.0f, 0.0f, static_cast<float>(m_ClientWidth), static_cast<float>(m_ClientHeight));
		m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());
	}
}

// IMGUI是否需要获取鼠标控制权
bool g_is_imgui_capture_mouse = false;

void GameApp::UpdateScene(float dt)
{
	 // 更新鼠标事件，获取相对偏移量
	const Mouse::State mouseState = m_pMouse->GetState();
	
	const Keyboard::State keyState = m_pKeyboard->GetState();
	m_KeyboardTracker.Update(keyState);

	if (m_CameraMode == CameraMode::FirstPerson || (m_CameraMode == CameraMode::ThirdPerson && keyState.IsKeyDown(Keyboard::LeftControl)))
	{
		if (keyState.IsKeyDown(Keyboard::W))
		{
			m_Player.Walk(dt * 6.0f);
		}
		if (keyState.IsKeyDown(Keyboard::S))
		{
			m_Player.Walk(dt * -6.0f);
		}
		if (keyState.IsKeyDown(Keyboard::A))
		{
			m_Player.Strafe(dt * -6.0f);
		}
		if (keyState.IsKeyDown(Keyboard::D))
		{
			m_Player.Strafe(dt * 6.0f);
		}
	}

	if (m_CameraMode == CameraMode::FirstPerson)
	{
		auto firstPersonCamera = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
		m_Player.AdjustPosition();
		//Transform& transform = m_Player.GetTransform();
		
		// 将摄像机位置限制在[-8.9, 8.9]x[-8.9, 8.9]x[0.0, 8.9]的区域内
		// 不允许穿地
		//XMFLOAT3 adjustedPos{};
		//XMStoreFloat3(&adjustedPos, XMVectorClamp(transform.GetPositionXM(), XMVectorSet(-8.9f, 0.0f, -8.9f, 0.0f), XMVectorReplicate(8.9f)));
		//transform.SetPosition(adjustedPos);
		//transform.LookTo(first_person_camera->GetLookAxis());
		// 第一人称摄像机距物体中心偏一点
		const XMFLOAT3 position = m_Player.GetTransform().GetPosition();
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
			g_is_imgui_capture_mouse = true;
		}
		else
		{
			m_pMouse->SetMode(Mouse::MODE_RELATIVE);
			g_is_imgui_capture_mouse = false;
		}
	}
	else if(m_CameraMode == CameraMode::ThirdPerson)
	{
		g_is_imgui_capture_mouse = false;
		auto thirdPersonCamera = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);
		m_Player.AdjustPosition();
		//Transform& transform = m_Player.GetTransform();

		//XMFLOAT3 adjustedPos{};
		//XMStoreFloat3(&adjustedPos, XMVectorClamp(transform.GetPositionXM(), XMVectorSet(-8.9f, 0.0f, -8.9f, 0.0f), XMVectorReplicate(8.9f)));
		//transform.SetPosition(adjustedPos);
		//transform.LookTo(third_person_camera->GetLookAxis());
		
		// 设置目标
		thirdPersonCamera->SetTarget(m_Player.GetTransform().GetPosition());
		// 绕物体旋转
		thirdPersonCamera->RotateX(static_cast<float>(mouseState.y) * dt * 2.5f);
		thirdPersonCamera->RotateY(static_cast<float>(mouseState.x) * dt * 2.5f);
		thirdPersonCamera->Approach(static_cast<float>(-mouseState.scrollWheelValue) / 120 * 1.0f);
	}

	// 更新观察矩阵
	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());
	m_BasicEffect.SetEyePos(m_pCamera->GetPositionXM());

	// 重置滚轮值
	m_pMouse->ResetScrollWheelValue();

	// 摄像机模式切换
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D1) && m_CameraMode != CameraMode::FirstPerson)
	{
		// 先保存摄像机之前的方向,这样子切换视角不会导致摄像机方向变化
		const XMFLOAT3 look = m_pCamera->GetLookAxis();
		const XMFLOAT3 up = m_pCamera->GetUpAxis();
		auto first_person_camera = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
		if (!first_person_camera)
		{
			first_person_camera.reset(new FirstPersonCamera);
			first_person_camera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = first_person_camera;
		}

		first_person_camera->LookTo(
			m_Player.GetTransform().GetPosition(),
			look,
			up
		);

		m_CameraMode = CameraMode::FirstPerson;
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D2) && m_CameraMode != CameraMode::ThirdPerson)
	{
		// 先保存摄像机之前的方向,这样子切换视角不会导致摄像机方向变化
		const XMFLOAT3 look = m_pCamera->GetLookAxis();
		const XMFLOAT3 up = m_pCamera->GetUpAxis();
		auto thirdPersonCamera = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);
		if (!thirdPersonCamera)
		{
			thirdPersonCamera.reset(new ThirdPersonCamera);
			thirdPersonCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = thirdPersonCamera;
		}
		
		thirdPersonCamera->SetTarget(m_Player.GetTransform().GetPosition(), true, look, up);
		thirdPersonCamera->SetDistance(8.0f);
		thirdPersonCamera->SetDistanceMinMax(3.0f, 20.0f);

		m_CameraMode = CameraMode::ThirdPerson;
	}

	// 退出程序，这里应向窗口发送销毁信息
	if (keyState.IsKeyDown(Keyboard::Escape))
	{
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
	}

	// 更新闪电动画
	static int currBoltFrame = 0;
	static float frameTime = 0.0f;
	m_BoltAnim.SetTexture(mBoltSRVs[currBoltFrame].Get());
	if (frameTime > 1.0f / 60)
	{
		currBoltFrame = (currBoltFrame + 1) % 60;
		frameTime -= 1.0f / 60;
	}
	frameTime += dt;
	
	m_ImguiPanel.LoadData(m_Player);
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	m_ImguiPanel.Draw();
	
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

	// ******************
	// 1. 给镜面反射区域写入值1到模板缓冲区
	//

	m_BasicEffect.SetWriteStencilOnly(m_pd3dImmediateContext.Get(), 1);

	m_Mirror.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	// ******************
	// 2. 绘制不透明的反射物体
	//

	// 开启反射绘制
	m_BasicEffect.SetReflectionState(true);	// 反射开启
	m_BasicEffect.SetRenderDefaultWithStencil(m_pd3dImmediateContext.Get(), 1);

	m_Walls[2].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_Walls[3].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_Walls[4].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_Floor.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	
	m_Player.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	// ******************
	// 3. 绘制不透明反射物体的阴影
	//

	m_Player.SetMaterial(m_ShadowMat);
	m_BasicEffect.SetShadowState(true);	// 反射开启，阴影开启			
	m_BasicEffect.SetRenderNoDoubleBlend(m_pd3dImmediateContext.Get(), 1);

	m_Player.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	// 恢复到原来的状态
	m_BasicEffect.SetShadowState(false);
	m_Player.SetMaterial(m_WoodCrateMat);
	

	// ******************
	// 4. 绘制需要混合的反射闪电动画和透明物体
	//

	m_BasicEffect.SetDrawBoltAnimNoDepthWriteWithStencil(m_pd3dImmediateContext.Get(), 1);
	m_BoltAnim.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	m_BasicEffect.SetReflectionState(false);		// 反射关闭
	m_BasicEffect.SetRenderAlphaBlendWithStencil(m_pd3dImmediateContext.Get(), 1);

	m_Mirror.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	
	// ******************
	// 5. 绘制不透明的正常物体
	//

	m_BasicEffect.SetRenderDefault(m_pd3dImmediateContext.Get());
	
	for (auto& wall : m_Walls)
	{
		wall.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	}
	m_Floor.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_Player.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	
	// ******************
	// 6. 绘制不透明正常物体的阴影
	//

	m_Player.SetMaterial(m_ShadowMat);
	m_BasicEffect.SetShadowState(true);	// 反射关闭，阴影开启
	m_BasicEffect.SetRenderNoDoubleBlend(m_pd3dImmediateContext.Get(), 0);

	m_Player.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	m_BasicEffect.SetShadowState(false);		// 阴影关闭
	m_Player.SetMaterial(m_WoodCrateMat);

	// ******************
	// 7. 绘制需要混合的闪电动画
	m_BasicEffect.SetDrawBoltAnimNoDepthWrite(m_pd3dImmediateContext.Get());
	m_BoltAnim.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	
	// 绘制Direct2D部分
	//
	if (m_pd2dRenderTarget != nullptr)
	{
		m_pd2dRenderTarget->BeginDraw();
		std::wstring text = L"基础特效\n"
			L"当前模式: ";
		if (m_CameraMode == CameraMode::ThirdPerson)
			text += L"第三人称";
		else
			text += L"第一人称";
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
	// ******************
	// 初始化游戏对象
	ComPtr<ID3D11ShaderResourceView> texture;
	Material material{};
	material.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);

	m_WoodCrateMat = material;
	m_ShadowMat.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_ShadowMat.diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	m_ShadowMat.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

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
	mBoltSRVs.assign(60, nullptr);
	wchar_t animFileName[50];
	// 初始化闪电
	for (int i = 1; i <= 60; ++i)
	{
		wsprintf(animFileName, L"Texture\\BoltAnim\\Bolt%03d.bmp", i);
		HR(CreateWICTextureFromFile(m_pd3dDevice.Get(), animFileName, nullptr, mBoltSRVs[static_cast<size_t>(i) - 1].GetAddressOf()));
	}
	m_BoltAnim.SetBuffer(m_pd3dDevice.Get(), Geometry::CreateCylinderNoCap(5.0f, 4.0f));
	// 抬起高度避免深度缓冲区资源争夺
	m_BoltAnim.GetTransform().SetPosition(0.0f, 2.01f, 0.0f);
	m_BoltAnim.SetMaterial(material);

	// 初始化玩家
	m_Player.init(m_pd3dDevice.Get());
	
	// 初始化地板
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Floor.SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(50.0f, 50.0f), XMFLOAT2(5.0f, 5.0f)));
	m_Floor.GetTransform().SetPosition(0.0f, -1.0f, 0.0f);
	m_Floor.SetTexture(texture.Get());
	m_Floor.SetMaterial(material);

	// 初始化墙体
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	// 这里控制墙体五个面的生成，0和1的中间位置用于放置镜面
	//     ____     ____
	//    /| 0 |   | 1 |\
	//   /4|___|___|___|2\
	//  /_/_ _ _ _ _ _ _\_\
	// | /       3       \ |
	// |/_________________\|
	//
	m_Walls[0].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(15.0f, 15.0f), XMFLOAT2(1.5f, 2.0f)));
	m_Walls[1].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(15.0f, 15.0f), XMFLOAT2(1.5f, 2.0f)));
	m_Walls[2].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(50.0f, 15.0f), XMFLOAT2(5.0f, 2.0f)));
	m_Walls[3].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(50.0f, 15.0f), XMFLOAT2(5.0f, 2.0f)));
	m_Walls[4].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(50.0f, 15.0f), XMFLOAT2(5.0f, 2.0f)));
	// 墙0
	m_Walls[0].GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
	m_Walls[0].GetTransform().SetPosition(-17.5f, 6.5f, 25.0f);
	// 墙1
	m_Walls[1].GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
	m_Walls[1].GetTransform().SetPosition(17.5f, 6.5f, 25.0f);
	// 墙2
	m_Walls[2].GetTransform().SetRotation(-XM_PIDIV2, XM_PIDIV2, 0.0f);
	m_Walls[2].GetTransform().SetPosition(25.0f, 6.5f, 0.0f);
	// 墙3
	m_Walls[3].GetTransform().SetRotation(-XM_PIDIV2, XM_PI, 0.0f);
	m_Walls[3].GetTransform().SetPosition(0.0f, 6.5f, -25.0f);
	// 墙4
	m_Walls[4].GetTransform().SetRotation(-XM_PIDIV2, -XM_PIDIV2, 0.0f);
	m_Walls[4].GetTransform().SetPosition(-25.0f, 6.5f, 0.0f);
	for (int i = 0; i < 5; ++i)
	{
		m_Walls[i].SetMaterial(material);
		m_Walls[i].SetTexture(texture.Get());
	}

	// 初始化镜面
	material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	material.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\ice.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Mirror.SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f, 15.0f), XMFLOAT2(1.0f, 1.0f)));
	m_Mirror.GetTransform().SetPosition(0.0f, 6.5f, 25.0f);
	m_Mirror.GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
	m_Mirror.SetTexture(texture.Get());
	m_Mirror.SetMaterial(material);

	// ******************
	// 初始化摄像机
	//
	
	auto camera = std::make_shared<ThirdPersonCamera>();
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, static_cast<float>(m_ClientWidth), static_cast<float>(m_ClientHeight));
	camera->SetTarget(XMFLOAT3(0.0f, 0.5f, 0.0f));
	camera->SetDistance(8.0f);
	camera->SetDistanceMinMax(3.0f, 20.0f);
	camera->SetRotationX(XM_PIDIV4);

	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());
	m_BasicEffect.SetEyePos(m_pCamera->GetPositionXM());
	
	// 初始化仅在窗口大小变动时修改的值
	m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
	m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());
	
	// ******************
	// 初始化不会变化的值
	//

	m_BasicEffect.SetReflectionMatrix(XMMatrixReflect(XMVectorSet(0.0f, 0.0f, -1.0f, 25.0f)));
	// 稍微高一点位置以显示阴影
	m_BasicEffect.SetShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 15.0f, -25.0f, 1.0f)));
	m_BasicEffect.SetRefShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 15.0f, 75.0f, 1.0f)));

	// 环境光
	m_BasicEffect.SetDirLight(
		0, 
		{
			XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
			XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
			XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
			XMFLOAT3(0.0f, -1.0f, 0.0f)
		}
	);
	// 灯光
	m_BasicEffect.SetPointLight(
		0, 
		{
			XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f),
			XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f),
			XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f),
			XMFLOAT3(0.0f, 10.0f, -10.0f),
			25.0f,
			XMFLOAT3(0.0f, 0.1f, 0.0f)
		}
	);

	// ******************
	// 设置调试对象名
	//
	m_BoltAnim.SetDebugObjectName("BoltAnim");
	m_Floor.SetDebugObjectName("Floor");
	m_Mirror.SetDebugObjectName("Mirror");
	m_Walls[0].SetDebugObjectName("Walls[0]");
	m_Walls[1].SetDebugObjectName("Walls[1]");
	m_Walls[2].SetDebugObjectName("Walls[2]");
	m_Walls[3].SetDebugObjectName("Walls[3]");
	m_Walls[4].SetDebugObjectName("Walls[4]");
	
	return true;
}
