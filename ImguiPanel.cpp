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
	ImGui::Text(u8"当前图形三轴方向: ");
	ImGui::Text(u8"\tLook: (%.3f, %.3f, %.3f)", look.x, look.y, look.z);
	ImGui::Text(u8"\tUp: (%.3f, %.3f, %.3f)", up.x, up.y, up.z);
	ImGui::Text(u8"\tRight: (%.3f, %.3f, %.3f)", right.x, right.y, right.z);

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
