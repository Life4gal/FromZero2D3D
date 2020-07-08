#include "ImguiPanel.h"

bool ImguiPanel::init(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* device_context)
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
	ImGui_ImplDX11_Init(device, device_context);

	return true;
}

void ImguiPanel::LoadData(const Player& player)
{
	m_direction = player.GetDirection();
	m_size = { player.m_tank.m_bodyWidth, player.m_tank.m_bodyLength, player.m_tank.m_bodyHeight };
	m_position = player.m_tank.GetPartObject().GetTransform().GetPosition();
}

void ImguiPanel::Draw() const
{
	//
	// 开始当前Dear ImGui帧渲染
	//
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	static bool show_demo_window = false;

	ImGui::Begin(u8"控制窗口");
	ImGui::Text(u8"当前方向为: ");
	ImGui::Text(u8"\t(vX = %.3f, vY = %.3f, vZ = %.3f)", m_direction.x, m_direction.y, m_direction.z);
	ImGui::Text(u8"车大小为: ");
	ImGui::Text(u8"\t(W = %.3f, L = %.3f, H = %.3f)", m_size.x, m_size.y, m_size.z);
	ImGui::Text(u8"当前位置为: ");
	ImGui::Text(u8"\t(X = %.3f, Y = %.3f, Z = %.3f)", m_position.x, m_position.y, m_position.z);

	ImGui::Checkbox(u8"显示演示窗口", &show_demo_window);

	ImGui::End();

	if (show_demo_window)
	{
		ImGui::ShowDemoWindow(&show_demo_window);
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

void ImguiPanel::shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
