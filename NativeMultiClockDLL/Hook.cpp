#include "Hook.h"
#include "ClockWindow.h"

HINSTANCE GlobalInstance = nullptr;
HHOOK InjectionHook = nullptr;
BOOL InjectionHookFirstRun = true;
HHOOK NewTaskbarHook = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (DLL_PROCESS_ATTACH == ul_reason_for_call)
	{
		GlobalInstance = (HINSTANCE)hModule;
		::DisableThreadLibraryCalls(GlobalInstance);
	}
	return TRUE;
}

MYHOOK_API BOOL Hook()
{
	HWND tray = ::FindWindow(L"Shell_TrayWnd", L"");
	if (tray == nullptr || !::IsWindow(tray))
	{
		return FALSE;
	}

	DWORD shell = ::GetWindowThreadProcessId(tray, nullptr);
	InjectionHook = SetWindowsHookEx(WH_CALLWNDPROC, HookInject, GlobalInstance, shell);
	NewTaskbarHook = ::SetWindowsHookEx(WH_CALLWNDPROCRET, HookNewTaskbar, GlobalInstance, shell);
	return InjectionHook != nullptr && NewTaskbarHook != nullptr;
}

static DWORD WINAPI ExitThreadProc(void *param)
{
	FreeLibraryAndExitThread(GlobalInstance, 0);
}

static void CALLBACK CloseClock(HWND tray, VOID* unused = nullptr)
{
	HWND hwnd = ::FindWindowEx(tray, nullptr, CLOCK_WINDOW_CLASS, nullptr);
	if (hwnd != nullptr)
	{
		PostMessage(hwnd, WM_CLOSE, 0x0, 0x0);
	}
}

MYHOOK_API BOOL Unhook()
{
	if (InjectionHook == nullptr)
	{
		return FALSE;
	}

	EnumHandler handler = { CloseClock, nullptr };
	::EnumWindows(EnumAllTaskbarsEx, (LPARAM)&handler);
	::Sleep(2000);
	::CreateThread(nullptr, 0, ExitThreadProc, nullptr, 0, nullptr);
	return ::UnhookWindowsHookEx(InjectionHook) && ::UnhookWindowsHookEx(NewTaskbarHook);
}

static void CALLBACK HookTaskbar(HWND tray, VOID* unused = nullptr)
{
	ClockWindow* multiClock = new ClockWindow();
	HWND hwnd = multiClock->Create(tray, nullptr, nullptr, WS_CHILD | WS_DISABLED, 0 | WS_EX_LAYERED, 0U, nullptr);

	multiClock->RepositionInTray(tray);
	multiClock->ShowWindow(SW_SHOW);
	multiClock->Refresh();
}

static LRESULT CALLBACK HookNewTaskbar(int code, WPARAM wParam, LPARAM lParam)
{
	if (code < 0)
	{
		return ::CallNextHookEx(NULL, code, wParam, lParam);
	}
	else
	{
		CWPRETSTRUCT* info = (CWPRETSTRUCT*)lParam;
		if (info != nullptr)
		{
			if (info->message == WM_CREATE)
			{
				wchar_t name[128];
				if (::GetClassName(info->hwnd, name, 128) > 0)
				{
					if (_wcsicmp(name, L"Shell_SecondaryTrayWnd") == 0)
					{
						HookTaskbar(info->hwnd);
					}
				}
			}
		}
		

		return ::CallNextHookEx(NULL, code, wParam, lParam);
	}
	
}

// RUNS IN OTHER THREAD / PROCESS!!!
static LRESULT CALLBACK HookInject(int code, WPARAM wParam, LPARAM lParam)
{
	if (InjectionHookFirstRun)
	{
		InjectionHookFirstRun = false;
		EnumHandler handler = { HookTaskbar, nullptr };
		::EnumWindows(EnumAllTaskbarsEx, (LPARAM)&handler);
	}
	return ::CallNextHookEx(nullptr, code, wParam, lParam);
}

MYHOOK_API BOOL CALLBACK EnumAllTaskbarsEx(HWND hwnd, LPARAM lParam)
{
	wchar_t name[128];
	if (::GetClassName(hwnd, name, 128) == 0)
	{
		return TRUE;
	}

	if (_wcsicmp(name, L"Shell_SecondaryTrayWnd") == 0)
	{
		EnumHandler* handler = (EnumHandler*)lParam;
		handler->func(hwnd, handler->param);
	}
	return TRUE;
}