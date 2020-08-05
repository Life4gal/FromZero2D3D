//--------------------------------------------------------------------------------------
// File: Keyboard.h
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
#include <cstdint>
#include <cassert>
// ReSharper disable once CppUnusedIncludeDirective
#include <exception>
#include <wrl/client.h>

namespace DirectX
{
	// ReSharper disable once CppClassCanBeFinal
	class Keyboard
	{
	public:
		Keyboard() noexcept(false);
		Keyboard(Keyboard&& moveFrom) noexcept;
		Keyboard& operator= (Keyboard&& moveFrom) noexcept;

		Keyboard(Keyboard const&) = delete;
		Keyboard& operator=(Keyboard const&) = delete;

		virtual ~Keyboard() = default;

		enum class Keys
		{
			NONE = 0,

			BACK = 0x8,
			TAB = 0x9,

			ENTER = 0xd,

			PAUSE = 0x13,
			CAPS_LOCK = 0x14,
			KANA = 0x15,

			KANJI = 0x19,

			ESCAPE = 0x1b,
			IME_CONVERT = 0x1c,
			IME_NO_CONVERT = 0x1d,

			SPACE = 0x20,
			PAGE_UP = 0x21,
			PAGE_DOWN = 0x22,
			END = 0x23,
			HOME = 0x24,
			LEFT = 0x25,
			UP = 0x26,
			RIGHT = 0x27,
			DOWN = 0x28,
			SELECT = 0x29,
			PRINT = 0x2a,
			EXECUTE = 0x2b,
			PRINT_SCREEN = 0x2c,
			INSERT = 0x2d,
#ifdef DELETE
#define TMP_DELETE = DELETE;  // NOLINT(cppcoreguidelines-macro-usage)
#undef DELETE
			DELETE = 0x2e,
// TODO 我们这里好像和 winnt.h 里面的DELETE名字上冲突了
#define DELETE TMP_DELETE  // NOLINT(cppcoreguidelines-macro-usage)
#undef TMP_DELETE
#endif
			HELP = 0x2f,
			D0 = 0x30,
			D1 = 0x31,
			D2 = 0x32,
			D3 = 0x33,
			D4 = 0x34,
			D5 = 0x35,
			D6 = 0x36,
			D7 = 0x37,
			D8 = 0x38,
			D9 = 0x39,

			A = 0x41,
			B = 0x42,
			C = 0x43,
			D = 0x44,
			E = 0x45,
			F = 0x46,
			G = 0x47,
			H = 0x48,
			I = 0x49,
			J = 0x4a,
			K = 0x4b,
			L = 0x4c,
			M = 0x4d,
			N = 0x4e,
			O = 0x4f,
			P = 0x50,
			Q = 0x51,
			R = 0x52,
			S = 0x53,
			T = 0x54,
			U = 0x55,
			V = 0x56,
			W = 0x57,
			X = 0x58,
			Y = 0x59,
			Z = 0x5a,
			LEFT_WINDOWS = 0x5b,
			RIGHT_WINDOWS = 0x5c,
			APPS = 0x5d,

			SLEEP = 0x5f,
			NUM_PAD0 = 0x60,
			NUM_PAD1 = 0x61,
			NUM_PAD2 = 0x62,
			NUM_PAD3 = 0x63,
			NUM_PAD4 = 0x64,
			NUM_PAD5 = 0x65,
			NUM_PAD6 = 0x66,
			NUM_PAD7 = 0x67,
			NUM_PAD8 = 0x68,
			NUM_PAD9 = 0x69,
			MULTIPLY = 0x6a,
			ADD = 0x6b,
			SEPARATOR = 0x6c,
			SUBTRACT = 0x6d,

			DECIMAL = 0x6e,
			DIVIDE = 0x6f,
			F1 = 0x70,
			F2 = 0x71,
			F3 = 0x72,
			F4 = 0x73,
			F5 = 0x74,
			F6 = 0x75,
			F7 = 0x76,
			F8 = 0x77,
			F9 = 0x78,
			F10 = 0x79,
			F11 = 0x7a,
			F12 = 0x7b,
			F13 = 0x7c,
			F14 = 0x7d,
			F15 = 0x7e,
			F16 = 0x7f,
			F17 = 0x80,
			F18 = 0x81,
			F19 = 0x82,
			F20 = 0x83,
			F21 = 0x84,
			F22 = 0x85,
			F23 = 0x86,
			F24 = 0x87,

			NUM_LOCK = 0x90,
			SCROLL = 0x91,

			LEFT_SHIFT = 0xa0,
			RIGHT_SHIFT = 0xa1,
			LEFT_CONTROL = 0xa2,
			RIGHT_CONTROL = 0xa3,
			LEFT_ALT = 0xa4,
			RIGHT_ALT = 0xa5,
			BROWSER_BACK = 0xa6,
			BROWSER_FORWARD = 0xa7,
			BROWSER_REFRESH = 0xa8,
			BROWSER_STOP = 0xa9,
			BROWSER_SEARCH = 0xaa,
			BROWSER_FAVORITES = 0xab,
			BROWSER_HOME = 0xac,
			VOLUME_MUTE = 0xad,
			VOLUME_DOWN = 0xae,
			VOLUME_UP = 0xaf,
			MEDIA_NEXT_TRACK = 0xb0,
			MEDIA_PREVIOUS_TRACK = 0xb1,
			MEDIA_STOP = 0xb2,
			MEDIA_PLAY_PAUSE = 0xb3,
			LAUNCH_MAIL = 0xb4,
			SELECT_MEDIA = 0xb5,
			LAUNCH_APPLICATION1 = 0xb6,
			LAUNCH_APPLICATION2 = 0xb7,

			OEM_SEMICOLON = 0xba,
			OEM_PLUS = 0xbb,
			OEM_COMMA = 0xbc,
			OEM_MINUS = 0xbd,
			OEM_PERIOD = 0xbe,
			OEM_QUESTION = 0xbf,
			OEM_TILDE = 0xc0,

			OEM_OPEN_BRACKETS = 0xdb,
			OEM_PIPE = 0xdc,
			OEM_CLOSE_BRACKETS = 0xdd,
			OEM_QUOTES = 0xde,
			OEM8 = 0xdf,

			OEM_BACKSLASH = 0xe2,

			PROCESS_KEY = 0xe5,

			OEM_COPY = 0xf2,
			OEM_AUTO = 0xf3,
			OEM_ENL_W = 0xf4,

			ATTN = 0xf6,
			CRSEL = 0xf7,
			EXSEL = 0xf8,
			ERASE_EOF = 0xf9,
			PLAY = 0xfa,
			ZOOM = 0xfb,

			PA1 = 0xfd,
			OEM_CLEAR = 0xfe,
		};

		struct State
		{
			bool reserved0 : 8;
			
			bool back : 1;              // VK_BACK, 0x8
			bool tab : 1;               // VK_TAB, 0x9
			
			bool reserved1 : 3;
			
			bool enter : 1;             // VK_RETURN, 0xD
			
			bool reserved2 : 2;
			bool reserved3 : 3;
			
			bool pause : 1;             // VK_PAUSE, 0x13
			bool capsLock : 1;          // VK_CAPITAL, 0x14
			bool kana : 1;              // VK_KANA, 0x15
			
			bool reserved4 : 2;
			bool reserved5 : 1;
			
			bool kanji : 1;             // VK_KANJI, 0x19
			
			bool reserved6 : 1;
			
			bool escape : 1;            // VK_ESCAPE, 0x1B
			bool imeConvert : 1;        // VK_CONVERT, 0x1C
			bool imeNoConvert : 1;      // VK_NONCONVERT, 0x1D
			
			bool reserved7 : 2;
			
			bool space : 1;             // VK_SPACE, 0x20
			bool pageUp : 1;            // VK_PRIOR, 0x21
			bool pageDown : 1;          // VK_NEXT, 0x22
			bool end : 1;               // VK_END, 0x23
			bool home : 1;              // VK_HOME, 0x24
			bool left : 1;              // VK_LEFT, 0x25
			bool up : 1;                // VK_UP, 0x26
			bool right : 1;             // VK_RIGHT, 0x27
			bool down : 1;              // VK_DOWN, 0x28
			bool select : 1;            // VK_SELECT, 0x29
			bool print : 1;             // VK_PRINT, 0x2A
			bool execute : 1;           // VK_EXECUTE, 0x2B
			bool printScreen : 1;       // VK_SNAPSHOT, 0x2C
			bool insert : 1;            // VK_INSERT, 0x2D
			// TODO oh~ no! 这个符合我命名习惯的 delete 居然是保留字,太糟糕了,实在是太糟糕了!
			bool Delete : 1;            // VK_DELETE, 0x2E
			bool help : 1;              // VK_HELP, 0x2F
			bool d0 : 1;                // 0x30
			bool d1 : 1;                // 0x31
			bool d2 : 1;                // 0x32
			bool d3 : 1;                // 0x33
			bool d4 : 1;                // 0x34
			bool d5 : 1;                // 0x35
			bool d6 : 1;                // 0x36
			bool d7 : 1;                // 0x37
			bool d8 : 1;                // 0x38
			bool d9 : 1;                // 0x39
			
			bool reserved8 : 6;
			bool reserved9 : 1;
			
			bool a : 1;                 // 0x41
			bool b : 1;                 // 0x42
			bool c : 1;                 // 0x43
			bool d : 1;                 // 0x44
			bool e : 1;                 // 0x45
			bool f : 1;                 // 0x46
			bool g : 1;                 // 0x47
			bool h : 1;                 // 0x48
			bool i : 1;                 // 0x49
			bool j : 1;                 // 0x4A
			bool k : 1;                 // 0x4B
			bool l : 1;                 // 0x4C
			bool m : 1;                 // 0x4D
			bool n : 1;                 // 0x4E
			bool o : 1;                 // 0x4F
			bool p : 1;                 // 0x50
			bool q : 1;                 // 0x51
			bool r : 1;                 // 0x52
			bool s : 1;                 // 0x53
			bool t : 1;                 // 0x54
			bool u : 1;                 // 0x55
			bool v : 1;                 // 0x56
			bool w : 1;                 // 0x57
			bool x : 1;                 // 0x58
			bool y : 1;                 // 0x59
			bool z : 1;                 // 0x5A
			bool leftWindows : 1;       // VK_LWIN, 0x5B
			bool rightWindows : 1;      // VK_RWIN, 0x5C
			bool apps : 1;              // VK_APPS, 0x5D
			
			bool reserved10 : 1;
			
			bool sleep : 1;             // VK_SLEEP, 0x5F
			bool numPad0 : 1;           // VK_NUMPAD0, 0x60
			bool numPad1 : 1;           // VK_NUMPAD1, 0x61
			bool numPad2 : 1;           // VK_NUMPAD2, 0x62
			bool numPad3 : 1;           // VK_NUMPAD3, 0x63
			bool numPad4 : 1;           // VK_NUMPAD4, 0x64
			bool numPad5 : 1;           // VK_NUMPAD5, 0x65
			bool numPad6 : 1;           // VK_NUMPAD6, 0x66
			bool numPad7 : 1;           // VK_NUMPAD7, 0x67
			bool numPad8 : 1;           // VK_NUMPAD8, 0x68
			bool numPad9 : 1;           // VK_NUMPAD9, 0x69
			bool multiply : 1;          // VK_MULTIPLY, 0x6A
			bool add : 1;               // VK_ADD, 0x6B
			bool separator : 1;         // VK_SEPARATOR, 0x6C
			bool subtract : 1;          // VK_SUBTRACT, 0x6D
			bool decimal : 1;           // VK_DECIMAL, 0x6E
			bool divide : 1;            // VK_DIVIDE, 0x6F
			bool f1 : 1;                // VK_F1, 0x70
			bool f2 : 1;                // VK_F2, 0x71
			bool f3 : 1;                // VK_F3, 0x72
			bool f4 : 1;                // VK_F4, 0x73
			bool f5 : 1;                // VK_F5, 0x74
			bool f6 : 1;                // VK_F6, 0x75
			bool f7 : 1;                // VK_F7, 0x76
			bool f8 : 1;                // VK_F8, 0x77
			bool f9 : 1;                // VK_F9, 0x78
			bool f10 : 1;               // VK_F10, 0x79
			bool f11 : 1;               // VK_F11, 0x7A
			bool f12 : 1;               // VK_F12, 0x7B
			bool f13 : 1;               // VK_F13, 0x7C
			bool f14 : 1;               // VK_F14, 0x7D
			bool f15 : 1;               // VK_F15, 0x7E
			bool f16 : 1;               // VK_F16, 0x7F
			bool f17 : 1;               // VK_F17, 0x80
			bool f18 : 1;               // VK_F18, 0x81
			bool f19 : 1;               // VK_F19, 0x82
			bool f20 : 1;               // VK_F20, 0x83
			bool f21 : 1;               // VK_F21, 0x84
			bool f22 : 1;               // VK_F22, 0x85
			bool f23 : 1;               // VK_F23, 0x86
			bool f24 : 1;               // VK_F24, 0x87
			
			bool reserved11 : 8;
			
			bool numLock : 1;           // VK_NUMLOCK, 0x90
			bool scroll : 1;            // VK_SCROLL, 0x91
			
			bool reserved12 : 6;
			bool reserved13 : 8;
			
			bool leftShift : 1;         // VK_LSHIFT, 0xA0
			bool rightShift : 1;        // VK_RSHIFT, 0xA1
			bool leftControl : 1;       // VK_LCONTROL, 0xA2
			bool rightControl : 1;      // VK_RCONTROL, 0xA3
			bool leftAlt : 1;           // VK_LMENU, 0xA4
			bool rightAlt : 1;          // VK_RMENU, 0xA5
			bool browserBack : 1;       // VK_BROWSER_BACK, 0xA6
			bool browserForward : 1;    // VK_BROWSER_FORWARD, 0xA7
			bool browserRefresh : 1;    // VK_BROWSER_REFRESH, 0xA8
			bool browserStop : 1;       // VK_BROWSER_STOP, 0xA9
			bool browserSearch : 1;     // VK_BROWSER_SEARCH, 0xAA
			bool browserFavorites : 1;  // VK_BROWSER_FAVORITES, 0xAB
			bool browserHome : 1;       // VK_BROWSER_HOME, 0xAC
			bool volumeMute : 1;        // VK_VOLUME_MUTE, 0xAD
			bool volumeDown : 1;        // VK_VOLUME_DOWN, 0xAE
			bool volumeUp : 1;          // VK_VOLUME_UP, 0xAF
			bool mediaNextTrack : 1;    // VK_MEDIA_NEXT_TRACK, 0xB0
			bool mediaPreviousTrack : 1;// VK_MEDIA_PREV_TRACK, 0xB1
			bool mediaStop : 1;         // VK_MEDIA_STOP, 0xB2
			bool mediaPlayPause : 1;    // VK_MEDIA_PLAY_PAUSE, 0xB3
			bool launchMail : 1;        // VK_LAUNCH_MAIL, 0xB4
			bool selectMedia : 1;       // VK_LAUNCH_MEDIA_SELECT, 0xB5
			bool launchApplication1 : 1;// VK_LAUNCH_APP1, 0xB6
			bool launchApplication2 : 1;// VK_LAUNCH_APP2, 0xB7
			
			bool reserved14 : 2;
			
			bool oemSemicolon : 1;      // VK_OEM_1, 0xBA
			bool oemPlus : 1;           // VK_OEM_PLUS, 0xBB
			bool oemComma : 1;          // VK_OEM_COMMA, 0xBC
			bool oemMinus : 1;          // VK_OEM_MINUS, 0xBD
			bool oemPeriod : 1;         // VK_OEM_PERIOD, 0xBE
			bool oemQuestion : 1;       // VK_OEM_2, 0xBF
			bool oemTilde : 1;          // VK_OEM_3, 0xC0
			
			bool reserved15 : 7;
			bool reserved16 : 8;
			bool reserved17 : 8;
			bool reserved18 : 3;
			
			bool oemOpenBrackets : 1;   // VK_OEM_4, 0xDB
			bool oemPipe : 1;           // VK_OEM_5, 0xDC
			bool oemCloseBrackets : 1;  // VK_OEM_6, 0xDD
			bool oemQuotes : 1;         // VK_OEM_7, 0xDE
			bool oem8 : 1;              // VK_OEM_8, 0xDF
			
			bool reserved19 : 2;
			
			bool oemBackslash : 1;      // VK_OEM_102, 0xE2
			
			bool reserved20 : 2;
			
			bool processKey : 1;        // VK_PROCESSKEY, 0xE5
			
			bool reserved21 : 2;
			bool reserved22 : 8;
			bool reserved23 : 2;
			
			bool oemCopy : 1;           // 0XF2
			bool oemAuto : 1;           // 0xF3
			bool oemEnlW : 1;           // 0xF4
			
			bool reserved24 : 1;
			
			bool attn : 1;              // VK_ATTN, 0xF6
			bool crsel : 1;             // VK_CRSEL, 0xF7
			bool exsel : 1;             // VK_EXSEL, 0xF8
			bool eraseEof : 1;          // VK_EREOF, 0xF9
			bool play : 1;              // VK_PLAY, 0xFA
			bool zoom : 1;              // VK_ZOOM, 0xFB
			
			bool reserved25 : 1;
			
			bool pa1 : 1;               // VK_PA1, 0xFD
			bool oemClear : 1;          // VK_OEM_CLEAR, 0xFE
			
			bool reserved26 : 1;

			[[nodiscard]] bool __cdecl IsKeyDown(Keys key) const
			{
				const auto localKey = static_cast<unsigned>(key);
				if (localKey >= 0 && localKey <= 0xfe)
				{
					const unsigned int bf = 1u << (localKey & 0x1f);
					const auto ptr = reinterpret_cast<const uint32_t*>(this);
					
					return (ptr[(localKey >> 5)] & bf) != 0;
				}
				return false;
			}

			[[nodiscard]] bool __cdecl IsKeyUp(Keys key) const
			{
				const auto localKey = static_cast<unsigned>(key);
				if (localKey >= 0 && localKey <= 0xfe)
				{
					const unsigned int bf = 1u << (localKey & 0x1f);
					const auto ptr = reinterpret_cast<const uint32_t*>(this);
					
					return (ptr[(localKey >> 5)] & bf) == 0;
				}
				return false;
			}
		};

		class KeyboardStateTracker
		{
		public:
			State m_released;
			State m_pressed;

#pragma prefast(suppress: 26495, "Reset() performs the initialization")
			KeyboardStateTracker() noexcept { Reset(); }  // NOLINT(cppcoreguidelines-pro-type-member-init, hicpp-member-init)

			void __cdecl Update(const State& state);

			void __cdecl Reset() noexcept;

			[[nodiscard]] bool __cdecl IsKeyPressed(const Keys key) const { return m_pressed.IsKeyDown(key); }
			[[nodiscard]] bool __cdecl IsKeyReleased(const Keys key) const { return m_released.IsKeyDown(key); }

			[[nodiscard]] State __cdecl GetLastState() const { return m_lastState; }

		private:
			State m_lastState;
		};

		// Retrieve the current state of the keyboard
		[[nodiscard]] State __cdecl GetState() const;

		// Reset the keyboard state
		void __cdecl Reset() const;

		// Feature detection
		[[nodiscard]] bool __cdecl IsConnected() const;

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP) && defined(WM_USER)
		static void __cdecl ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);
#endif

		// Singleton
		static Keyboard& __cdecl Get();

	private:
		// Private implementation.
		class Impl;

		std::unique_ptr<Impl> m_pImpl;
	};
}
