#pragma once
#include "imgui.h"      // IMGUI_IMPL_API

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <tchar.h>

IMGUI_IMPL_API bool     ImGui_ImplWin32_Init(HWND hwnd);
IMGUI_IMPL_API void     ImGui_ImplWin32_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplWin32_NewFrame();
