//--------------------------------------------------------------------------------------
// File: Mouse.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//
// Modified By: life4gal(NiceT)(MIT License)
// 
//--------------------------------------------------------------------------------------

#include "Mouse.h"
#include <functional>


using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct HandleCloser { void operator()(const HANDLE h) const { if (h) CloseHandle(h); } };

typedef std::unique_ptr<void, HandleCloser> ScopedHandle;

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)

class Mouse::Impl
{
public:
	explicit Impl(Mouse* owner)
		:
		m_state{},
		m_owner(owner),
		m_window(nullptr),
		m_mode(Mode::MODE_ABSOLUTE),
		m_lastX(0),
		m_lastY(0),
		m_relativeX(INT32_MAX),
		m_relativeY(INT32_MAX),
		m_inFocus(true)
	{
		if (g_mouse)
		{
			throw std::exception("Mouse is a singleton");
		}

		g_mouse = this;

		m_scrollWheelValue.reset(CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE));
		m_relativeRead.reset(CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE));
		m_absoluteMode.reset(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
		m_relativeMode.reset(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
		if (!m_scrollWheelValue
			|| !m_relativeRead
			|| !m_absoluteMode
			|| !m_relativeMode)
		{
			throw std::exception("CreateEventEx");
		}
	}

	~Impl()
	{
		g_mouse = nullptr;
	}

	void GetState(State& state) const
	{
		memcpy(&state, &m_state, sizeof(State));
		state.positionMode = m_mode;

		DWORD result = WaitForSingleObjectEx(m_scrollWheelValue.get(), 0, FALSE);
		if (result == WAIT_FAILED)
			throw std::exception("WaitForSingleObjectEx");

		if (result == WAIT_OBJECT_0)
		{
			state.scrollWheelValue = 0;
		}

		if (state.positionMode == Mode::MODE_RELATIVE)
		{
			result = WaitForSingleObjectEx(m_relativeRead.get(), 0, FALSE);

			if (result == WAIT_FAILED)
				throw std::exception("WaitForSingleObjectEx");

			if (result == WAIT_OBJECT_0)
			{
				state.x = 0;
				state.y = 0;
			}
			else
			{
				SetEvent(m_relativeRead.get());
			}
		}
	}

	void ResetScrollWheelValue() const
	{
		SetEvent(m_scrollWheelValue.get());
	}

	void SetMode(const Mode mode) const
	{
		if (m_mode == mode)
			return;

		SetEvent((mode == Mode::MODE_ABSOLUTE) ? m_absoluteMode.get() : m_relativeMode.get());

		assert(m_window != nullptr);

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_HOVER;
		tme.hwndTrack = m_window;
		tme.dwHoverTime = 1;
		if (!TrackMouseEvent(&tme))
		{
			throw std::exception("TrackMouseEvent");
		}
	}

	static bool IsConnected()
	{
		return GetSystemMetrics(SM_MOUSEPRESENT) != 0;
	}

	bool IsVisible() const
	{
		if (m_mode == Mode::MODE_RELATIVE)
			return false;

		CURSORINFO info = { sizeof(CURSORINFO), 0, nullptr, {} };
		if (!GetCursorInfo(&info))
		{
			throw std::exception("GetCursorInfo");
		}

		return (info.flags & CURSOR_SHOWING) != 0;
	}

	void SetVisible(const bool visible) const
	{
		if (m_mode == Mode::MODE_RELATIVE)
			return;

		CURSORINFO info = { sizeof(CURSORINFO), 0, nullptr, {} };
		if (!GetCursorInfo(&info))
		{
			throw std::exception("GetCursorInfo");
		}

		if (((info.flags & CURSOR_SHOWING) != 0) != visible)
		{
			ShowCursor(visible);
		}
	}

	void SetWindow(const HWND window)
	{
		if (m_window == window)
			return;

		assert(window != nullptr);

		RAWINPUTDEVICE rid;
		rid.usUsagePage = 0x1 /* HID_USAGE_PAGE_GENERIC */;
		rid.usUsage = 0x2 /* HID_USAGE_GENERIC_MOUSE */;
		rid.dwFlags = RIDEV_INPUTSINK;
		rid.hwndTarget = window;
		if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
		{
			throw std::exception("RegisterRawInputDevices");
		}

		m_window = window;
	}

	State           m_state;

	Mouse*          m_owner;

	static Impl* g_mouse;

private:
	HWND            m_window;
	Mode            m_mode;

	ScopedHandle    m_scrollWheelValue;
	ScopedHandle    m_relativeRead;
	ScopedHandle    m_absoluteMode;
	ScopedHandle    m_relativeMode;

	int             m_lastX;
	int             m_lastY;
	int             m_relativeX;
	int             m_relativeY;

	bool            m_inFocus;

	friend void Mouse::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

	void ClipToWindow() const
	{
		assert(m_window != nullptr);

		RECT rect;
		GetClientRect(m_window, &rect);

		POINT ul;
		ul.x = rect.left;
		ul.y = rect.top;

		POINT lr;
		lr.x = rect.right;
		lr.y = rect.bottom;

		MapWindowPoints(m_window, nullptr, &ul, 1);
		MapWindowPoints(m_window, nullptr, &lr, 1);

		rect.left = ul.x;
		rect.top = ul.y;

		rect.right = lr.x;
		rect.bottom = lr.y;

		ClipCursor(&rect);
	}
};

Mouse::Impl* Mouse::Impl::g_mouse = nullptr;

// ReSharper disable once CppParameterMayBeConst
void Mouse::SetWindow(HWND window) const
{
	m_pImpl->SetWindow(window);
}

void Mouse::ProcessMessage(const UINT message, const WPARAM wParam, const LPARAM lParam)
{
	auto pImpl = Impl::g_mouse;

	if (!pImpl)
		return;

	HANDLE events[3];
	events[0] = pImpl->m_scrollWheelValue.get();
	events[1] = pImpl->m_absoluteMode.get();
	events[2] = pImpl->m_relativeMode.get();
	
	switch (WaitForMultipleObjectsEx(_countof(events), events, FALSE, 0, FALSE))
	{
	case WAIT_OBJECT_0:
		pImpl->m_state.scrollWheelValue = 0;
		ResetEvent(events[0]);
		break;

	case (WAIT_OBJECT_0 + 1):
	{
		pImpl->m_mode = Mode::MODE_ABSOLUTE;
		ClipCursor(nullptr);

		POINT point;
		point.x = pImpl->m_lastX;
		point.y = pImpl->m_lastY;

		// We show the cursor before moving it to support Remote Desktop
		ShowCursor(TRUE);

		if (MapWindowPoints(pImpl->m_window, nullptr, &point, 1))
		{
			SetCursorPos(point.x, point.y);
		}
		pImpl->m_state.x = pImpl->m_lastX;
		pImpl->m_state.y = pImpl->m_lastY;

		break;
	}
	case (WAIT_OBJECT_0 + 2):
	{
		ResetEvent(pImpl->m_relativeRead.get());

		pImpl->m_mode = Mode::MODE_RELATIVE;
		pImpl->m_state.x = pImpl->m_state.y = 0;
		pImpl->m_relativeX = INT32_MAX;
		pImpl->m_relativeY = INT32_MAX;

		ShowCursor(FALSE);

		pImpl->ClipToWindow();

		break;
	}
	case WAIT_FAILED:
		throw std::exception("WaitForMultipleObjectsEx");
		
	// 按道理说如果不是上面三个结果就应该是失败,但是我们不能确定
	default:;
	}

	switch (message)
	{
	case WM_ACTIVATEAPP:
		if (wParam)
		{
			pImpl->m_inFocus = true;

			if (pImpl->m_mode == Mode::MODE_RELATIVE)
			{
				pImpl->m_state.x = pImpl->m_state.y = 0;

				ShowCursor(FALSE);

				pImpl->ClipToWindow();
			}
		}
		else
		{
			const int scrollWheel = pImpl->m_state.scrollWheelValue;
			memset(&pImpl->m_state, 0, sizeof(State));
			pImpl->m_state.scrollWheelValue = scrollWheel;

			pImpl->m_inFocus = false;
		}
		return;

	case WM_INPUT:
		if (pImpl->m_inFocus && pImpl->m_mode == Mode::MODE_RELATIVE)
		{
			RAWINPUT raw;
			UINT rawSize = sizeof(raw);

			const UINT resultData = GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &raw, &rawSize, sizeof(RAWINPUTHEADER));
			if (resultData == UINT(-1))
			{
				throw std::exception("GetRawInputData");
			}

			if (raw.header.dwType == RIM_TYPEMOUSE)
			{
				if (!(raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
				{
					pImpl->m_state.x = raw.data.mouse.lLastX;
					pImpl->m_state.y = raw.data.mouse.lLastY;

					ResetEvent(pImpl->m_relativeRead.get());
				}
				else if (raw.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP)
				{
					// This is used to make Remote Desktop session work
					const int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
					const int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

					const int x = static_cast<int>(float(raw.data.mouse.lLastX) / 65535.0f * width);
					const int y = static_cast<int>(float(raw.data.mouse.lLastY) / 65535.0f * height);

					if (pImpl->m_relativeX == INT32_MAX)
					{
						pImpl->m_state.x = pImpl->m_state.y = 0;
					}
					else
					{
						pImpl->m_state.x = x - pImpl->m_relativeX;
						pImpl->m_state.y = y - pImpl->m_relativeY;
					}

					pImpl->m_relativeX = x;
					pImpl->m_relativeY = y;

					ResetEvent(pImpl->m_relativeRead.get());
				}
			}
		}
		return;

	case WM_MOUSEMOVE:
		break;

	case WM_LBUTTONDOWN:
		pImpl->m_state.leftButton = true;
		break;

	case WM_LBUTTONUP:
		pImpl->m_state.leftButton = false;
		break;

	case WM_RBUTTONDOWN:
		pImpl->m_state.rightButton = true;
		break;

	case WM_RBUTTONUP:
		pImpl->m_state.rightButton = false;
		break;

	case WM_MBUTTONDOWN:
		pImpl->m_state.middleButton = true;
		break;

	case WM_MBUTTONUP:
		pImpl->m_state.middleButton = false;
		break;

	case WM_MOUSEWHEEL:
		pImpl->m_state.scrollWheelValue += GET_WHEEL_DELTA_WPARAM(wParam);
		return;

	case WM_XBUTTONDOWN:
		switch (GET_XBUTTON_WPARAM(wParam))
		{
		case XBUTTON1:
			pImpl->m_state.xButton1 = true;
			break;

		case XBUTTON2:
			pImpl->m_state.xButton2 = true;
			break;
		default:;
		}
		break;

	case WM_XBUTTONUP:
		switch (GET_XBUTTON_WPARAM(wParam))
		{
		case XBUTTON1:
			pImpl->m_state.xButton1 = false;
			break;

		case XBUTTON2:
			pImpl->m_state.xButton2 = false;
			break;
		default:;
		}
		break;

	case WM_MOUSEHOVER:
		break;

	default:
		// Not a mouse message, so exit
		return;
	}

	if (pImpl->m_mode == Mode::MODE_ABSOLUTE)
	{
		// All mouse messages provide a new pointer position
		const int xPos = static_cast<short>(LOWORD(lParam)); // GET_X_LPARAM(lParam);
		const int yPos = static_cast<short>(HIWORD(lParam)); // GET_Y_LPARAM(lParam);

		pImpl->m_state.x = pImpl->m_lastX = xPos;
		pImpl->m_state.y = pImpl->m_lastY = yPos;
	}
}

#endif

#pragma warning( disable : 4355 )

// Public constructor.
Mouse::Mouse() noexcept(false)
	: m_pImpl(std::make_unique<Impl>(this))
{
}


// Move constructor.
Mouse::Mouse(Mouse&& moveFrom) noexcept
	: m_pImpl(std::move(moveFrom.m_pImpl))
{
	m_pImpl->m_owner = this;
}

// Move assignment.
Mouse& Mouse::operator= (Mouse&& moveFrom) noexcept
{
	m_pImpl = std::move(moveFrom.m_pImpl);
	m_pImpl->m_owner = this;
	return *this;
}

// Public destructor.
Mouse::~Mouse()
{
}

Mouse::State Mouse::GetState() const
{
	State state;
	m_pImpl->GetState(state);
	return state;
}

void Mouse::ResetScrollWheelValue() const
{
	m_pImpl->ResetScrollWheelValue();
}

void Mouse::SetMode(const Mode mode) const
{
	m_pImpl->SetMode(mode);
}

bool Mouse::IsConnected() const
{
	return m_pImpl->IsConnected();
}

bool Mouse::IsVisible() const
{
	return m_pImpl->IsVisible();
}

void Mouse::SetVisible(const bool visible) const
{
	m_pImpl->SetVisible(visible);
}

Mouse& Mouse::Get()
{
	if (!Impl::g_mouse || !Impl::g_mouse->m_owner)
		throw std::exception("Mouse is a singleton");

	return *Impl::g_mouse->m_owner;
}

//======================================================================================
// ButtonStateTracker
//======================================================================================

namespace 
{
#ifdef BUTTONSTATE_USEMACRO
	// 我很讨厌胶水宏,但是看上去我们好像没有什么好的办法将它消灭
	#define UPDATE_BUTTON_STATE(field) field = static_cast<ButtonState>( ( !!state.field ) | ( ( !!state.field ^ !!lastState.field ) << 1 ) );
#endif
	
	/*
	enum class Button
	{
		MIDDLE_BUTTON,
		RIGHT_BUTTON,
		X_BUTTON1,
		X_BUTTON2
	};
	
	template <Button WhichButton, Mouse::ButtonStateTracker::ButtonState& Field, const Mouse::State& State, const Mouse::State& LastState>
	void UpdateButtonState()
	{
		if constexpr (WhichButton == Button::MIDDLE_BUTTON)
		{
			Field = static_cast<Mouse::ButtonStateTracker::ButtonState>((!!State.middleButton) | ((!!State.middleButton ^ !!LastState.middleButton) << 1));
		}
		else if constexpr (WhichButton == Button::RIGHT_BUTTON)
		{
			Field = static_cast<Mouse::ButtonStateTracker::ButtonState>((!!State.rightButton) | ((!!State.rightButton ^ !!LastState.rightButton) << 1));
		}
		else if constexpr (WhichButton == Button::X_BUTTON1)
		{
			Field = static_cast<Mouse::ButtonStateTracker::ButtonState>((!!State.xButton1) | ((!!State.xButton1 ^ !!LastState.xButton1) << 1));
		}
		else if constexpr (WhichButton == Button::X_BUTTON2)
		{
			Field = static_cast<Mouse::ButtonStateTracker::ButtonState>((!!State.xButton2) | ((!!State.xButton2 ^ !!LastState.xButton2) << 1));
		}
	}
	*/
}

void Mouse::ButtonStateTracker::Update(const State& state)
{
#ifdef BUTTONSTATE_USEMACRO
	UPDATE_BUTTON_STATE(leftButton);
	assert((!state.leftButton && !lastState.leftButton) == (leftButton == ButtonState::UP));
	assert((state.leftButton && lastState.leftButton) == (leftButton == ButtonState::HELD));
	assert((!state.leftButton && lastState.leftButton) == (leftButton == ButtonState::RELEASED));
	assert((state.leftButton && !lastState.leftButton) == (leftButton == ButtonState::PRESSED));
	UPDATE_BUTTON_STATE(middleButton);
	UPDATE_BUTTON_STATE(rightButton);
	UPDATE_BUTTON_STATE(xButton1);
	UPDATE_BUTTON_STATE(xButton2);
	lastState = state;
#else
	m_leftButton = static_cast<ButtonState>(state.leftButton | (state.leftButton ^ m_lastState.leftButton << 1));
	assert((!state.leftButton && !m_lastState.leftButton) == (m_leftButton == ButtonState::UP));
	assert((state.leftButton && m_lastState.leftButton) == (m_leftButton == ButtonState::HELD));
	assert((!state.leftButton && m_lastState.leftButton) == (m_leftButton == ButtonState::RELEASED));
	assert((state.leftButton && !m_lastState.leftButton) == (m_leftButton == ButtonState::PRESSED));
	m_middleButton = static_cast<ButtonState>(state.middleButton | (state.middleButton ^ m_lastState.middleButton << 1));
	m_rightButton = static_cast<ButtonState>(state.rightButton | (state.rightButton ^ m_lastState.rightButton << 1));
	m_xButton1 = static_cast<ButtonState>(state.xButton1 | (state.xButton1 ^ m_lastState.xButton1 << 1));
	m_xButton2 = static_cast<ButtonState>(state.xButton2 | (state.xButton2 ^ m_lastState.xButton2 << 1));
	m_lastState = state;
#endif
}

void Mouse::ButtonStateTracker::Reset() noexcept
{
	memset(this, 0, sizeof(ButtonStateTracker));
}
