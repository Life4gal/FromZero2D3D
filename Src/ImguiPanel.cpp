#include "ImguiPanel.h"

using namespace DirectX;

// 这几个函数只在这里使用,所以就搬过来了
namespace 
{
	// 方向光斜率
	int* g_slopeIndex = nullptr;
	// 开启调试模式
	bool* g_enableDebug = nullptr;
	// 深度值以灰度形式显示
	bool* g_grayMode = nullptr;
	
	// 能不能使用鼠标,用于消息处理
	bool g_isImGuiCanUseKBandMouse = false;
	// 如果IMGUI使用了键鼠,其他地方则不响应键鼠操作
#ifdef IMGUI_USED_KB_AND_MOUSE
	bool g_isImGuiUsedKBandMouse = false;
#endif
	
	// Win32 Data
	HWND                 g_hWnd = nullptr;
	INT64                g_time = 0;
	INT64                g_ticksPerSecond = 0;
	ImGuiMouseCursor     g_lastMouseCursor = ImGuiMouseCursor_COUNT;
	
	bool	ImGui_ImplWin32_Init(HWND hWnd);
	void	ImGui_ImplWin32_Shutdown();
	bool	ImGui_ImplWin32_UpdateMouseCursor();
	void	ImGui_ImplWin32_UpdateMousePos();
	void	ImGui_ImplWin32_NewFrame();
}

// ReSharper disable once CppParameterMayBeConst
bool ImguiPanel::Init(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	// 设置Dear ImGui上下文
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImFontConfig fontConfig;
	fontConfig.OversampleH = 2;
	fontConfig.OversampleV = 1;
	fontConfig.PixelSnapH = true;
	fontConfig.GlyphOffset.y -= 1.0f;      // Move everything by 1 pixels up
	fontConfig.GlyphExtraSpacing.x = 1.0f; // Increase spacing between characters
	// 微软雅黑-常规
	io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 18.0f, &fontConfig, io.Fonts->GetGlyphRangesChineseFull());

	// 设置Dear ImGui界面风格
	ImGui::StyleColorsDark();

	// 设置平台/渲染器的绑定
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device, deviceContext);

	return true;
}

void ImguiPanel::LoadData(int* slopeIndex, bool* enableDebug, bool* grayMode)
{
	g_slopeIndex = slopeIndex;
	g_enableDebug = enableDebug;
	g_grayMode = grayMode;
}

void ImguiPanel::Draw()
{
	//
	// 开始当前Dear ImGui帧渲染
	//
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	

	ImGui::Begin("控制窗口");

	static const char* slopeItem[] = { "1.0", "1.5", "2.0", "3.0", "4.0" };
	ImGui::Combo("方向光斜率", g_slopeIndex, slopeItem, 5);

	ImGui::Checkbox("开启调试模式", g_enableDebug);
	if(*g_enableDebug)
	{
		static const char* grayModeItem[] = { "灰度", "单通道" };
		// true -> 1 , false -> 0
		static int grayMode = *g_grayMode;
		// TODO 之前直接使用 reinterpret_cast<int*>(g_grayMode) 造成越界修改到了 g_slopeIndex 的值(将 g_grayMode 置为0(false)时会将 g_slopeIndex 也修改为0(方向光斜率变为1.0))
		ImGui::Combo("色显示", &grayMode, grayModeItem, 2);
		*g_grayMode = grayMode;
	}




	ImGui::End();

	//
	// 完成剩余的3D渲染
	//
	ImGui::Render();
}

// ReSharper disable once CppParameterMayBeConst
LRESULT ImguiPanel::ImGuiWndProc(HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	if (ImGui::GetCurrentContext() == nullptr)
		return 0;

	// g_isImGuiCanUseKBandMouse默认是false的,而可以改变他的地方必定是在 ImGui::NewFrame() 之后,
	// 此时调用 ImGui::IsAnyWindowHovered() 或 ImGui::IsAnyItemHovered() 是安全的
	// TODO 不论是单独的 !ImGui::IsAnyWindowHovered() 还是 !ImGui::IsAnyItemHovered() 都有问题
	// 前者不能响应点击物件,后者不能响应拖动窗体,一起用才行
	if (!g_isImGuiCanUseKBandMouse || !ImGui::IsAnyWindowHovered() && !ImGui::IsAnyItemHovered())
		return 0;

	ImGuiIO& io = ImGui::GetIO();
	switch (msg)
	{
	case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
	{
		int button = 0;
		if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
		if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
		if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
		if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
		if (!ImGui::IsAnyMouseDown() && GetCapture() == nullptr)
			SetCapture(hWnd);

		io.MouseDown[button] = true;

#ifdef IMGUI_USED_KB_AND_MOUSE
		g_isImGuiUsedKBandMouse = true;
#endif

		return 0;
	}
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	{
		int button = 0;
		if (msg == WM_LBUTTONUP) { button = 0; }
		if (msg == WM_RBUTTONUP) { button = 1; }
		if (msg == WM_MBUTTONUP) { button = 2; }
		if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
		io.MouseDown[button] = false;
		if (!ImGui::IsAnyMouseDown() && GetCapture() == hWnd)
			ReleaseCapture();

#ifdef IMGUI_USED_KB_AND_MOUSE
		g_isImGuiUsedKBandMouse = true;
#endif

		return 0;
	}
	case WM_MOUSEWHEEL:
		io.MouseWheel += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
#ifdef IMGUI_USED_KB_AND_MOUSE
		g_isImGuiUsedKBandMouse = true;
#endif
		return 0;
	case WM_MOUSEHWHEEL:
		io.MouseWheelH += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
#ifdef IMGUI_USED_KB_AND_MOUSE
		g_isImGuiUsedKBandMouse = true;
#endif
		return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam < 256)
		{
			io.KeysDown[wParam] = true;
#ifdef IMGUI_USED_KB_AND_MOUSE
			g_isImGuiUsedKBandMouse = true;
#endif
		}
		return 0;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (wParam < 256)
		{
			io.KeysDown[wParam] = false;
#ifdef IMGUI_USED_KB_AND_MOUSE
			g_isImGuiUsedKBandMouse = true;
#endif
		}
		return 0;
	case WM_CHAR:
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacterUTF16(static_cast<unsigned short>(wParam));
		return 0;
	case WM_SETCURSOR:
		if (LOWORD(lParam) == HTCLIENT && ImGui_ImplWin32_UpdateMouseCursor())
			return 1;
		return 0;
	default:
		return 0;
	}
}

void ImguiPanel::SetPanelCanUseKBandMouse(const bool canUse)
{
	g_isImGuiCanUseKBandMouse = canUse;
}

#ifdef IMGUI_USED_KB_AND_MOUSE
bool ImguiPanel::IsPanelUsedKBandMouse()
{
	// 如果 g_isImGuiUsedKBandMouse 为 false 则直接返回 false,如果为 true 则返回 true 并将其置为 false
	// TODO 不确定 g_isImGuiUsedKBandMouse = false 返回的是 true 还是 false, 好像是 false 唉
	return g_isImGuiUsedKBandMouse && (!(g_isImGuiUsedKBandMouse = false));
}
#endif

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

namespace
{
	// Functions
	// ReSharper disable once CppParameterMayBeConst
	bool    ImGui_ImplWin32_Init(HWND hWnd)
	{
		if (!QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&g_ticksPerSecond)))
			return false;
		if (!QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&g_time)))
			return false;

		// Setup back-end capabilities flags
		g_hWnd = hWnd;
		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
		io.BackendPlatformName = "imgui_impl_win32";
		io.ImeWindowHandle = hWnd;

		// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
		io.KeyMap[ImGuiKey_Tab] = VK_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
		io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
		io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
		io.KeyMap[ImGuiKey_Home] = VK_HOME;
		io.KeyMap[ImGuiKey_End] = VK_END;
		io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
		io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
		io.KeyMap[ImGuiKey_Space] = VK_SPACE;
		io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
		io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
		io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
		io.KeyMap[ImGuiKey_A] = 'A';
		io.KeyMap[ImGuiKey_C] = 'C';
		io.KeyMap[ImGuiKey_V] = 'V';
		io.KeyMap[ImGuiKey_X] = 'X';
		io.KeyMap[ImGuiKey_Y] = 'Y';
		io.KeyMap[ImGuiKey_Z] = 'Z';

		return true;
	}

	void    ImGui_ImplWin32_Shutdown()
	{
		g_hWnd = nullptr;
	}

	bool ImGui_ImplWin32_UpdateMouseCursor()
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
			return false;

		const ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
		if (imguiCursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
		{
			// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
			SetCursor(nullptr);
		}
		else
		{
			// Show OS mouse cursor
			LPTSTR win32Cursor = IDC_ARROW;
			switch (imguiCursor)
			{
			case ImGuiMouseCursor_Arrow:        win32Cursor = IDC_ARROW; break;
			case ImGuiMouseCursor_TextInput:    win32Cursor = IDC_IBEAM; break;
			case ImGuiMouseCursor_ResizeAll:    win32Cursor = IDC_SIZEALL; break;
			case ImGuiMouseCursor_ResizeEW:     win32Cursor = IDC_SIZEWE; break;
			case ImGuiMouseCursor_ResizeNS:     win32Cursor = IDC_SIZENS; break;
			case ImGuiMouseCursor_ResizeNESW:   win32Cursor = IDC_SIZENESW; break;
			case ImGuiMouseCursor_ResizeNWSE:   win32Cursor = IDC_SIZENWSE; break;
			case ImGuiMouseCursor_Hand:         win32Cursor = IDC_HAND; break;
			case ImGuiMouseCursor_NotAllowed:   win32Cursor = IDC_NO; break;
			default:;
			}
			SetCursor(::LoadCursor(nullptr, win32Cursor));
		}

		return true;
	}

	void ImGui_ImplWin32_UpdateMousePos()
	{
		ImGuiIO& io = ImGui::GetIO();

		// Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
		if (io.WantSetMousePos)
		{
			POINT pos = { static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y) };
			ClientToScreen(g_hWnd, &pos);
			SetCursorPos(pos.x, pos.y);
		}

		// Set mouse position
		io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
		POINT pos;

		// ReSharper disable once CppLocalVariableMayBeConst
		if (HWND activeWindow = GetForegroundWindow())
			if (activeWindow == g_hWnd || IsChild(activeWindow, g_hWnd))
				if (GetCursorPos(&pos) && ScreenToClient(g_hWnd, &pos))
					io.MousePos = ImVec2(static_cast<float>(pos.x), static_cast<float>(pos.y));
	}

	void    ImGui_ImplWin32_NewFrame()
	{
		ImGuiIO& io = ImGui::GetIO();
		IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

		// Setup display size (every frame to accommodate for window resizing)
		RECT rect;
		GetClientRect(g_hWnd, &rect);
		io.DisplaySize = ImVec2(static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top));

		// Setup time step
		INT64 currentTime;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentTime));
		io.DeltaTime = static_cast<float>(currentTime - g_time) / g_ticksPerSecond;
		g_time = currentTime;

		// Read keyboard modifiers inputs
		io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
		io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
		io.KeySuper = false;
		// io.KeysDown[], io.MousePos, io.MouseDown[], io.MouseWheel: filled by the WndProc handler below.

		// Update OS mouse position
		ImGui_ImplWin32_UpdateMousePos();

		// Update OS mouse cursor with the cursor requested by imgui
		const ImGuiMouseCursor mouseCursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
		if (g_lastMouseCursor != mouseCursor)
		{
			g_lastMouseCursor = mouseCursor;
			ImGui_ImplWin32_UpdateMouseCursor();
		}
	}
}
