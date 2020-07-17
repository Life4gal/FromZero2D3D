#ifndef IMGUIPANEL_H
#define IMGUIPANEL_H

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "Tank.h"

class ImguiPanel
{
public:
	// 初始化 IMGUI
	static bool Init(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	// 获取需要的数据
	void LoadData(const Tank& player);
	// 绘制 IMGUI 面板
	void Draw() const;
	// 呈现 IMGUI 面板,必须在 其他2D/3D部分渲染完毕 之后且在 SwapChain->Present 前调用,不然不会显示面板
	static void Present();
	// 关闭 IMGUI
	static void Shutdown();

	DirectX::XMFLOAT3 m_direction{};
	DirectX::XMFLOAT3 m_position{};
	
	DirectX::XMFLOAT3 m_barrelDirection{};
	DirectX::XMFLOAT3 m_barrelPosition{};
};

#endif