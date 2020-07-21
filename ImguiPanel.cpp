#include "ImguiPanel.h"

using namespace DirectX;

// ReSharper disable once CppParameterMayBeConst
bool ImguiPanel::Init(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	// 设置Dear ImGui上下文
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImFontConfig font_config;
	font_config.OversampleH = 2;
	font_config.OversampleV = 1;
	font_config.PixelSnapH = true;
	font_config.GlyphOffset.y -= 1.0f;      // Move everything by 1 pixels up
	font_config.GlyphExtraSpacing.x = 1.0f; // Increase spacing between characters
	// 微软雅黑-常规
	io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 18.0f, &font_config, io.Fonts->GetGlyphRangesChineseFull());

	// 设置Dear ImGui界面风格
	ImGui::StyleColorsDark();

	// 设置平台/渲染器的绑定
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device, deviceContext);

	return true;
}

void ImguiPanel::LoadData(const Player& player)
{
	XMStoreFloat3(&m_direction, player.m_tank.m_tankMainBody.GetTransform().GetForwardAxisVector());
	m_position = player.m_tank.GetPosition();

	XMMATRIX barrelLocalToWorldMatrix = player.m_tank.GetBarrelLocalToWorldMatrix();

	XMStoreFloat3(&m_barrelDirection, barrelLocalToWorldMatrix.r[1]);
	XMStoreFloat3(&m_barrelPosition, barrelLocalToWorldMatrix.r[3]);
}

void ImguiPanel::Draw() const
{
	//
	// 开始当前Dear ImGui帧渲染
	//
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	static bool showDemoWindow = false;

	ImGui::Begin(u8"控制窗口");
	ImGui::Text(u8"车身方向为: ");
	ImGui::Text(u8"\t(vX = %.3f, vY = %.3f, vZ = %.3f)", m_direction.x, m_direction.y, m_direction.z);
	ImGui::Text(u8"车身位置为: ");
	ImGui::Text(u8"\t(X = %.3f, Y = %.3f, Z = %.3f)", m_position.x, m_position.y, m_position.z);
	
	ImGui::Text(u8"炮管方向为: ");
	ImGui::Text(u8"\t(X = %.3f, Y = %.3f, Z = %.3f)", m_barrelDirection.x, m_barrelDirection.y, m_barrelDirection.z);
	ImGui::Text(u8"炮管位置为: ");
	ImGui::Text(u8"\t(X = %.3f, Y = %.3f, Z = %.3f)", m_barrelPosition.x, m_barrelPosition.y, m_barrelPosition.z);

	ImGui::Checkbox(u8"显示演示窗口", &showDemoWindow);

	ImGui::End();

	if (showDemoWindow)
	{
		ImGui::ShowDemoWindow(&showDemoWindow);
	}

	//
	// 完成剩余的3D渲染
	//
	ImGui::Render();
}

void ImguiPanel::Present()
{
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ImguiPanel::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
