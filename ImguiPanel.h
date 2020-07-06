#ifndef IMGUIPANEL_H
#define IMGUIPANEL_H

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// 包含这个文件是因为d3dapp里也包含了这个文件,而这里需要的仅仅是 HWND 这个结构的定义而已
#include <wrl/client.h>
#include <DirectXMath.h>

class ImguiPanel
{
public:
	// 初始化 IMGUI
	static bool init(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* device_context);
	// 绘制 IMGUI 面板
	void Draw() const;
	// 呈现 IMGUI 面板,必须在 其他2D/3D部分渲染完毕 之后且在 SwapChain->Present 前调用,不然不会显示面板
	static void Present();
	// 关闭 IMGUI
	static void shutdown();

	DirectX::XMFLOAT3 m_direction{};
	DirectX::XMFLOAT3 m_size{};
	DirectX::XMFLOAT3 m_position{};
};

#endif