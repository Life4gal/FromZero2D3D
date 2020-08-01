#include "imgui_impl_win32.h"

// Win32 Data
static HWND                 g_hWnd = nullptr;
static INT64                g_time = 0;
static INT64                g_ticksPerSecond = 0;
static ImGuiMouseCursor     g_lastMouseCursor = ImGuiMouseCursor_COUNT;

// Functions
// ReSharper disable once CppParameterMayBeConst
bool    ImGui_ImplWin32_Init(HWND hwnd)
{
    if (!QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&g_ticksPerSecond)))
        return false;
    if (!QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&g_time)))
        return false;

    // Setup back-end capabilities flags
    g_hWnd = hwnd;
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_win32";
    io.ImeWindowHandle = hwnd;

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

static bool ImGui_ImplWin32_UpdateMouseCursor()
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
	        default: ;
        }
        SetCursor(::LoadCursor(nullptr, win32Cursor));
    }
	
    return true;
}

static void ImGui_ImplWin32_UpdateMousePos()
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
    ::GetClientRect(g_hWnd, &rect);
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

extern bool g_isImGuiCanUseKBandMouse;
// 如果IMGUI使用了键鼠,其他地方则不响应键鼠操作
bool g_isImGuiUsedKBandMouse = false;

// ReSharper disable once CppParameterMayBeConst
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	if (ImGui::GetCurrentContext() == nullptr)
		return 0;

	// g_isImGuiCanUseKBandMouse是false的,而可以改变他的地方必定是在 ImGui::NewFrame() 之后,
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

            g_isImGuiUsedKBandMouse = true;
			
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
			if (!ImGui::IsAnyMouseDown() && ::GetCapture() == hWnd)
				ReleaseCapture();

            g_isImGuiUsedKBandMouse = true;
			
			return 0;
		}
	case WM_MOUSEWHEEL:
		io.MouseWheel += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
        g_isImGuiUsedKBandMouse = true;
		return 0;
	case WM_MOUSEHWHEEL:
		io.MouseWheelH += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
        g_isImGuiUsedKBandMouse = true;
		return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam < 256)
		{
			io.KeysDown[wParam] = true;
			g_isImGuiUsedKBandMouse = true;
		}
		return 0;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (wParam < 256)
		{
			io.KeysDown[wParam] = false;
			g_isImGuiUsedKBandMouse = true;
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
