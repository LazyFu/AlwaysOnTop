#include <Windows.h>
#include <shellapi.h>
#include <vector>
#include <string>
#include <algorithm>

#define WND_CLASS_NAME L"AlwaysOnTop"

#define HOTKEY_ID 1
#define HOTKEY_MOD (MOD_ALT | MOD_SHIFT)  // Alt+Shift
#define HOTKEY_VK 0x54  // T

#define WM_APP_TRAYMSG (WM_APP+1)
#define TRAY_ICON_ID 1
#define ID_MENU_EXIT 1001

struct WindowInfo
{
	HWND hWnd;
	wchar_t title[256];
};
NOTIFYICONDATAW g_trayData;
std::vector<WindowInfo> g_windowList;
std::vector<HWND> g_pinnedWindows;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void InitTrayIcon(HWND hWnd);
void RemoveTrayIcon(HWND hWnd);
void ShowContextMenu(HWND hWnd);
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEXW wc = {0};
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = WND_CLASS_NAME;
	if(!RegisterClassExW(&wc))
	{
		MessageBoxW(NULL, L"Failed to register window class", L"Error", MB_ICONERROR);
		return 1;
	}

	HWND hWnd = CreateWindowExW(
	                0, WND_CLASS_NAME, L"Always On Top", 0, 0, 0, 0, 0,
	                HWND_MESSAGE, NULL, hInstance, NULL
	            );
	if (!hWnd)
	{
		MessageBoxW(NULL, L"Failed to create window", L"Error", MB_ICONERROR);
		return 1;
	}

	if (!RegisterHotKey(hWnd, HOTKEY_ID, HOTKEY_MOD, HOTKEY_VK))
	{
		MessageBoxW(hWnd, L"Failed to register hotkey", L"Error", MB_ICONERROR);
		return 1;
	}

	InitTrayIcon(hWnd);

	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_HOTKEY:
			if (wParam == HOTKEY_ID)
			{
				HWND gfWnd = GetForegroundWindow();
				if (gfWnd)
				{
					bool isTopmost = (GetWindowLongPtr(gfWnd, GWL_EXSTYLE) & WS_EX_TOPMOST);
					if (isTopmost)
					{
						SetWindowPos(gfWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
						auto it = std::find(g_pinnedWindows.begin(), g_pinnedWindows.end(), gfWnd);
						if (it != g_pinnedWindows.end())
						{
							g_pinnedWindows.erase(it);
						}
					}
					else
					{
						SetWindowPos(gfWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
						auto it = std::find(g_pinnedWindows.begin(), g_pinnedWindows.end(), gfWnd);
						if (it == g_pinnedWindows.end())
						{
							g_pinnedWindows.push_back(gfWnd);
						}
					}
				}
			}
			break;

		case WM_APP_TRAYMSG:
			switch (lParam)
			{
				case WM_RBUTTONUP:
					ShowContextMenu(hWnd);
					break;
				case WM_LBUTTONUP:
					ShowContextMenu(hWnd);
					break;
				default:
					break;
			}
			break;

		case WM_COMMAND:
			if (LOWORD(wParam) == ID_MENU_EXIT)
			{
				DestroyWindow(hWnd);
			}
			break;

		case WM_DESTROY:
			for (HWND hPinnedWnd : g_pinnedWindows)
			{
				if (IsWindow(hPinnedWnd))
				{
					SetWindowPos(hPinnedWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				}
			}
			UnregisterHotKey(hWnd, HOTKEY_ID);
			RemoveTrayIcon(hWnd);
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProcW(hWnd, message, wParam, lParam);
	}
	return 0;
}

void InitTrayIcon(HWND hWnd)
{
	g_trayData.cbSize = sizeof(NOTIFYICONDATAW);
	g_trayData.hWnd = hWnd;
	g_trayData.uID = TRAY_ICON_ID;
	g_trayData.uFlags = NIF_INFO | NIF_MESSAGE | NIF_TIP;
	g_trayData.uCallbackMessage = WM_APP_TRAYMSG;

	g_trayData.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	wcscpy_s(g_trayData.szTip, L"Always On Top - Alt+Shift+T");
	Shell_NotifyIconW(NIM_ADD, &g_trayData);
}

void RemoveTrayIcon(HWND hWnd)
{
	Shell_NotifyIconW(NIM_DELETE, &g_trayData);
}

void ShowContextMenu(HWND hWnd)
{
	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{
		g_windowList.clear();
		EnumWindows(EnumWindowsProc, 0);

		for (const auto& window : g_windowList)
		{
			std::wstring menuItem = window.title;
			if(GetWindowLongPtr(window.hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
			{
				menuItem = L"(Pinned) "+ menuItem;
			}
			AppendMenuW(hMenu, MF_STRING | MF_GRAYED, 0, menuItem.c_str());
		}
		AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
		AppendMenuW(hMenu, MF_STRING, ID_MENU_EXIT, L"Exit");

		POINT pt;
		GetCursorPos(&pt);
		SetForegroundWindow(hWnd);
		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_RIGHTALIGN, pt.x, pt.y, 0, hWnd, NULL);
		DestroyMenu(hMenu);
	}
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	if (IsWindowVisible(hWnd) && GetWindowTextLengthW(hWnd) > 0)
	{
		WindowInfo info;
		info.hWnd = hWnd;
		GetWindowTextW(hWnd, info.title, sizeof(info.title) / sizeof(info.title[0]));
		g_windowList.push_back(info);
	}
	return TRUE;
}