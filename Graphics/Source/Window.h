#pragma once

#include "Util.h"

namespace dx
{
	class Win32Window
	{
	public:
		Win32Window();

		int MessageLoop();

		HWND GetHWnd() const 
		{ 
			return m_hWnd; 
		}

		Signal<> OnTick;
		Signal<int, int> OnResize;

	private:
		HWND m_hWnd;
		// Raw Input buffer
		std::vector<char> m_buf;

		static constexpr const wchar_t* WINDOW_CLASS = L"MyWindowClass";

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	};
}