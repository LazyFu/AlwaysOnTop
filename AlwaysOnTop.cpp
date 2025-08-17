#include <Windows.h>

#define HOTKEY_ID 1
#define HOTKEY_MOD (MOD_CONTROL | MOD_SHIFT)  // Ctrl+Shift
#define HOTKEY_VK 0x54  // T

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	if (!RegisterHotKey(NULL, HOTKEY_ID, HOTKEY_MOD, HOTKEY_VK))
	{
		return 1;
	}

	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_HOTKEY)
		{
			HWND hWnd = GetForegroundWindow();
			if (hWnd)
			{
				bool isTopmost = (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST);

				SetWindowPos(hWnd,
				             isTopmost ? HWND_NOTOPMOST : HWND_TOPMOST,
				             0, 0, 0, 0,
				             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
		}
	}

	UnregisterHotKey(NULL, HOTKEY_ID);
	return 0;
}