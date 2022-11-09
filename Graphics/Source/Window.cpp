#include "stdafx.h"

#include "Window.h"

#include "Keyboard.h"
#include "Mouse.h"

namespace
{
	constexpr int GetX(LPARAM lParam)
	{
		return static_cast<int>(static_cast<short>(LOWORD(lParam)));
	}

	constexpr int GetY(LPARAM lParam)
	{
		return static_cast<int>(static_cast<short>(HIWORD(lParam)));
	}
}

namespace dx
{
	Win32Window::Win32Window() :
		m_hWnd(nullptr)
	{
		WNDCLASSEX wcex{};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.lpfnWndProc = &WndProc;
		wcex.lpszClassName = L"MyWindowClass";
		wcex.style = CS_HREDRAW | CS_VREDRAW;

		auto rc = RegisterClassEx(&wcex);
		if (!rc)
		{
			throw std::runtime_error("Failed to register window class");
		}

		m_hWnd = CreateWindowEx(0, WINDOW_CLASS, L"App", WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
			nullptr, nullptr, nullptr, this);
		if (!m_hWnd)
		{
			throw std::runtime_error("Failed to create window");
		}
		SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		// Register for Raw Input
		RAWINPUTDEVICE rid{};
		rid.usUsagePage = 1;
		rid.usUsage = 2;
		if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
		{
			throw std::runtime_error("Failed to register for Raw Input");
		}

		ShowWindow(m_hWnd, SW_MAXIMIZE);
	}

	int Win32Window::MessageLoop()
	{
		MSG msg{};
		while (msg.message != WM_QUIT)
		{
			// Get all pending messages before we update the game
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) && msg.message != WM_QUIT)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			
			OnTick.Send();
		}
		return 0;
	}

	LRESULT Win32Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		static bool mouseOverWindow = false;
		static bool resizing = false;
		static bool fullscreen = false;
		static RECT prevRect{};
		auto* instance = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		switch (uMsg)
		{
		case WM_DESTROY:
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_SIZE:
		{
			if (!resizing)
			{
				instance->OnResize.Send(LOWORD(lParam), HIWORD(lParam));
			}
			return 0;
		}
		case WM_ENTERSIZEMOVE:
		{
			resizing = true;
			return 0;
		}
		case WM_EXITSIZEMOVE:
		{
			resizing = false;
			RECT rect;
			GetClientRect(hWnd, &rect);
			instance->OnResize.Send(rect.right - rect.left, rect.bottom - rect.top);
			return 0;
		}
		case WM_SYSKEYDOWN:
		{
			// Fullscreen toggle (ALT + ENTER)
			if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
			{
				if (fullscreen)
				{
					// Go windowed
					SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
					SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);
					ShowWindow(hWnd, SW_SHOWNORMAL);
					SetWindowPos(hWnd, HWND_TOP, prevRect.left, prevRect.top,
						prevRect.right - prevRect.left, prevRect.bottom - prevRect.top,
						SWP_NOZORDER | SWP_FRAMECHANGED);
					fullscreen = false;
				}
				else
				{
					// Go fullscreen
					GetWindowRect(hWnd, &prevRect);
					SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
					SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
					SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
					ShowWindow(hWnd, SW_SHOWMAXIMIZED);
					fullscreen = true;
				}
			}
			return 0;
		}
		case WM_KEYDOWN:
		{
			WORD code = LOWORD(wParam);
			Keyboard::GetInstance().OnKey(IntToEnum<Key>(code), KeyEvent::ePress);
			return 0;
		}
		case WM_KEYUP:
		{
			WORD code = LOWORD(wParam);
			Keyboard::GetInstance().OnKey(IntToEnum<Key>(code), KeyEvent::eRelease);
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			POINT pt{ GetX(lParam), GetY(lParam) };
			Mouse::GetInstance().OnMove(pt.x, pt.y);

			if (!mouseOverWindow)
			{
				mouseOverWindow = true;
				Mouse::GetInstance().OnEnter();

				TRACKMOUSEEVENT tme{};
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hWnd;
				TrackMouseEvent(&tme);
			}

			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			Mouse::GetInstance().OnButton(Button::eLeft, ButtonEvent::eClick);
			return 0;
		}
		case WM_LBUTTONUP:
		{
			Mouse::GetInstance().OnButton(Button::eLeft, ButtonEvent::eRelease);
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			Mouse::GetInstance().OnButton(Button::eRight, ButtonEvent::eClick);
			return 0;
		}
		case WM_RBUTTONUP:
		{
			Mouse::GetInstance().OnButton(Button::eRight, ButtonEvent::eRelease);
			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			short delta = GET_WHEEL_DELTA_WPARAM(wParam);
			Mouse::GetInstance().OnScroll(delta);
			return 0;
		}
		case WM_MOUSELEAVE:
		{
			mouseOverWindow = false;
			Mouse::GetInstance().OnLeave();
			return 0;
		}
		case WM_INPUT:
		{
			UINT size;
			UINT rc = GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, 
				nullptr, &size, sizeof(RAWINPUTHEADER));
			if (rc != 0)
			{
				return 0;
			}
			instance->m_buf.resize(size);

			rc = GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT,
				instance->m_buf.data(), &size, sizeof(RAWINPUTHEADER));
			if (rc != size)
			{
				return 0;
			}

			auto* ri = reinterpret_cast<RAWINPUT*>(instance->m_buf.data());
			if ((ri->header.dwType == RIM_TYPEMOUSE)
				&& ((ri->data.mouse.lLastX != 0) || (ri->data.mouse.lLastY != 0)))
			{
				Mouse::GetInstance().OnRawInput(ri->data.mouse.lLastX, ri->data.mouse.lLastY);
			}
			return 0;
		}
		case WM_MENUCHAR:
		{
			return MAKELRESULT(0, MNC_CLOSE);
		}
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}