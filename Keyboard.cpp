//--------------------------------------------------------------------------------------
// File: Keyboard.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//
// // Modified By: life4gal(NiceT)(MIT License)
// 
//--------------------------------------------------------------------------------------

#include "Keyboard.h"

using namespace DirectX;

// ReSharper disable once CppParameterMayBeConst
struct HandleCloser { void operator()(HANDLE h) const { if (h) CloseHandle(h); } };

typedef std::unique_ptr<void, HandleCloser> ScopedHandle;

static_assert(sizeof(Keyboard::State) == (256 / 8), "Size mismatch for State");

namespace
{
	void KeyDown(const int key, Keyboard::State& state)
	{
		if (key < 0 || key > 0xfe)
			return;

		const unsigned int bf = 1u << (key & 0x1f);
		
		const auto ptr = reinterpret_cast<uint32_t*>(&state);
		ptr[(key >> 5)] |= bf;
	}

	void KeyUp(const int key, Keyboard::State& state)
	{
		if (key < 0 || key > 0xfe)
			return;

		const unsigned int bf = 1u << (key & 0x1f);
		const auto ptr = reinterpret_cast<uint32_t*>(&state);
		
		ptr[(key >> 5)] &= ~bf;
	}
}


#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)


class Keyboard::Impl  // NOLINT(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
{
public:
	explicit Impl(Keyboard* owner) :
		m_mState{},
		m_mOwner(owner)
	{
		if (g_keyboard)
		{
			throw std::exception("Keyboard is a singleton");
		}

		g_keyboard = this;
	}

	~Impl()
	{
		g_keyboard = nullptr;
	}

	void GetState(State& state) const
	{
		memcpy(&state, &m_mState, sizeof(State));
	}

	void Reset()
	{
		memset(&m_mState, 0, sizeof(State));
	}

	static bool IsConnected()
	{
		return true;
	}

	State           m_mState;
	Keyboard*       m_mOwner;

	static Impl* g_keyboard;
};

Keyboard::Impl* Keyboard::Impl::g_keyboard = nullptr;

void Keyboard::ProcessMessage(const UINT message, const WPARAM wParam, const LPARAM lParam)
{
	auto pImpl = Impl::g_keyboard;

	if (!pImpl)
		return;

	bool down = false;

	switch (message)
	{
	case WM_ACTIVATEAPP:
		pImpl->Reset();
		return;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		down = true;
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		break;

	default:
		return;
	}

	int vk = static_cast<int>(wParam);
	switch (vk)
	{
	case VK_SHIFT:
		vk = MapVirtualKey((lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
		if (!down)
		{
			// Workaround to ensure left vs. right shift get cleared when both were pressed at same time
			KeyUp(VK_LSHIFT, pImpl->m_mState);
			KeyUp(VK_RSHIFT, pImpl->m_mState);
		}
		break;

	case VK_CONTROL:
		vk = (lParam & 0x01000000) ? VK_RCONTROL : VK_LCONTROL;
		break;

	case VK_MENU:
		vk = (lParam & 0x01000000) ? VK_RMENU : VK_LMENU;
		break;
	default:;
	}

	if (down)
	{
		KeyDown(vk, pImpl->m_mState);
	}
	else
	{
		KeyUp(vk, pImpl->m_mState);
	}
}

#endif

#pragma warning( disable : 4355 )

// Public constructor.
Keyboard::Keyboard() noexcept(false)
	: m_pImpl(std::make_unique<Impl>(this))
{
}

// Move constructor.
Keyboard::Keyboard(Keyboard&& moveFrom) noexcept
	: m_pImpl(std::move(moveFrom.m_pImpl))
{
	m_pImpl->m_mOwner = this;
}

// Move assignment.
Keyboard& Keyboard::operator= (Keyboard&& moveFrom) noexcept
{
	m_pImpl = std::move(moveFrom.m_pImpl);
	m_pImpl->m_mOwner = this;
	return *this;
}

Keyboard::State Keyboard::GetState() const
{
	State state{};
	m_pImpl->GetState(state);
	return state;
}

void Keyboard::Reset() const
{
	m_pImpl->Reset();
}

bool Keyboard::IsConnected() const
{
	return m_pImpl->IsConnected();
}

Keyboard& Keyboard::Get()
{
	if (!Impl::g_keyboard || !Impl::g_keyboard->m_mOwner)
		throw std::exception("Keyboard is a singleton");

	return *Impl::g_keyboard->m_mOwner;
}

//======================================================================================
// KeyboardStateTracker
//======================================================================================

void Keyboard::KeyboardStateTracker::Update(const State& state)
{
	auto currPtr = reinterpret_cast<const uint32_t*>(&state);
	auto prevPtr = reinterpret_cast<const uint32_t*>(&m_lastState);
	auto releasedPtr = reinterpret_cast<uint32_t*>(&m_released);
	auto pressedPtr = reinterpret_cast<uint32_t*>(&m_pressed);
	for (size_t j = 0; j < (256 / 32); ++j)
	{
		*pressedPtr = *currPtr & ~(*prevPtr);
		*releasedPtr = ~(*currPtr) & *prevPtr;

		++currPtr;
		++prevPtr;
		++releasedPtr;
		++pressedPtr;
	}

	m_lastState = state;
}

void Keyboard::KeyboardStateTracker::Reset() noexcept
{
	memset(this, 0, sizeof(KeyboardStateTracker));
}
