//--------------------------------------------------------------------------------------
// File: Mouse.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//
// Modified By: life4gal(NiceT)(MIT License)
// 小幅度改动
// 
//--------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <memory>
#include <cassert>
// ReSharper disable once CppUnusedIncludeDirective
#include <exception>
#include <wrl/client.h>

namespace DirectX
{
	// ReSharper disable once CppClassCanBeFinal
	class Mouse
	{
	public:
		Mouse() noexcept(false);
		Mouse(Mouse&& moveFrom) noexcept;
		Mouse& operator= (Mouse&& moveFrom) noexcept;

		Mouse(Mouse const&) = delete;
		Mouse& operator=(Mouse const&) = delete;

		virtual ~Mouse() = default;

		enum class Mode
		{
			MODE_ABSOLUTE = 0,
			MODE_RELATIVE = 1,
		};

		struct State
		{
			bool    leftButton;
			bool    middleButton;
			bool    rightButton;
			bool    xButton1;
			bool    xButton2;
			int     x;
			int     y;
			int     scrollWheelValue;
			Mode    positionMode;
		};

		class ButtonStateTracker
		{
		public:
			enum class ButtonState
			{
				UP = 0,         // Button is up
				HELD = 1,       // Button is held down
				RELEASED = 2,   // Button was just released
				PRESSED = 3,    // Button was just pressed
			};
			
// TODO warning C4805: “^”: 在操作中将类型“const bool”与类型“int”混合不安全,我宣布这一轮macro取得胜利
#define BUTTONSTATE_USEMACRO
			
#ifdef BUTTONSTATE_USEMACRO
			// ReSharper disable once CppInconsistentNaming
			ButtonState leftButton;
			// ReSharper disable once CppInconsistentNaming
			ButtonState middleButton;
			// ReSharper disable once CppInconsistentNaming
			ButtonState rightButton;
			// ReSharper disable once CppInconsistentNaming
			ButtonState xButton1;
			// ReSharper disable once CppInconsistentNaming
			ButtonState xButton2;
#else
			ButtonState m_leftButton;
			ButtonState m_middleButton;
			ButtonState m_rightButton;
			ButtonState m_xButton1;
			ButtonState m_xButton2;
#endif

#pragma prefast(suppress: 26495, "Reset() performs the initialization")
			ButtonStateTracker() noexcept { Reset(); }  // NOLINT(cppcoreguidelines-pro-type-member-init, hicpp-member-init)

			void __cdecl Update(const State& state);

			void __cdecl Reset() noexcept;

			[[nodiscard]] State __cdecl GetLastState() const
			{
#ifdef BUTTONSTATE_USEMACRO
				return lastState;
#else
				return m_lastState;
#endif
			}

		private:
#ifdef BUTTONSTATE_USEMACRO
			// ReSharper disable once CppInconsistentNaming
			State lastState;
#else
			State m_lastState;
#endif

// 其他地方不需要这个坏东西了
#undef BUTTONSTATE_USEMACRO
		};

		// Retrieve the current state of the mouse
		[[nodiscard]] State __cdecl GetState() const;

		// Resets the accumulated scroll wheel value
		void __cdecl ResetScrollWheelValue() const;

		// Sets mouse mode (defaults to absolute)
		void __cdecl SetMode(Mode mode) const;

		// Feature detection
		[[nodiscard]] bool __cdecl IsConnected() const;

		// Cursor visibility
		[[nodiscard]] bool __cdecl IsVisible() const;
		void __cdecl SetVisible(bool visible) const;

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP) && defined(WM_USER)
		void __cdecl SetWindow(HWND window) const;
		static void __cdecl ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);
#endif
		// Singleton
		static Mouse& __cdecl Get();

	private:
		// Private implementation.
		class Impl;

		std::unique_ptr<Impl> m_pImpl;
	};
}
