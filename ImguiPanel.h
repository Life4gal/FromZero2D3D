#ifndef IMGUIPANEL_H
#define IMGUIPANEL_H

#include "ImGui/imgui_impl_dx11.h"

#include "d3dApp.h"

class ImguiPanel
{
public:
	// 初始化 IMGUI
	static bool Init(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	// 绘制 IMGUI 面板
	static void Draw();
	// 我们将WIN32处理消息的方式移动到我们自己的IMGUI面板里
	static LRESULT ImGuiWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	// 呈现 IMGUI 面板,必须在 其他2D/3D部分渲染完毕 之后且在 SwapChain->Present 前调用,不然不会显示面板

	// 设置IMGUI能不能使用键鼠
	static void SetPanelCanUseKBandMouse(bool canUse);
	// IMGUI 是否使用过了键鼠,主要是为了让 DX 的键鼠不会在操作 IMGUI 的时候同步响应
#ifdef IMGUI_USED_KB_AND_MOUSE
	static bool IsPanelUsedKBandMouse();
#endif
	
	static void Present();
	// 关闭 IMGUI
	static void Shutdown();
};

#endif
