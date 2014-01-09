#include "Hook.h"
#include "ClockWindow.h"
#include "Error.h"

HINSTANCE GlobalInstance = nullptr;
HHOOK InjectionHook = nullptr;
BOOL InjectionHookFirstRun = true;

// Warning: used in both processes!
HWND GlobalMainTaskbar = nullptr;
HWND GetMainTaskbar()
{
	if (GlobalMainTaskbar == nullptr || !::IsWindow(GlobalMainTaskbar))
	{
		GlobalMainTaskbar = ::FindWindow(TASKBAR_MAIN_CLASS, nullptr);
	}
	return ::IsWindow(GlobalMainTaskbar) ? GlobalMainTaskbar : nullptr;
}

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
	HWND mainTaskbar = GetMainTaskbar();
	if (mainTaskbar == nullptr)
	{
		return FALSE;
	}
	
	DWORD shell = ::GetWindowThreadProcessId(mainTaskbar, nullptr);
	InjectionHook = ::SetWindowsHookEx(WH_CALLWNDPROC, HookInject, GlobalInstance, shell);
	return InjectionHook != nullptr;
}

MYHOOK_API BOOL UnhookInjectionHook()
{
	if (InjectionHook == nullptr)
	{
		return FALSE;
	}

	BOOL result = ::UnhookWindowsHookEx(InjectionHook);
	if (result)
	{
		InjectionHook = nullptr;
	}
	return result;
}

static UINT NewTaskbarMessage = ::RegisterWindowMessage(L"NativeMultiClock.MyButton.NewTaskbarMessage");
static UINT UnhookMessage = ::RegisterWindowMessage(L"NativeMultiClock.MyButton.UnhookMessage");

MYHOOK_API BOOL Unhook()
{
	UnhookInjectionHook();

	HWND mainTaskbar = GetMainTaskbar();
	if (mainTaskbar == nullptr)
	{
		return FALSE;
	}

	::PostMessage(mainTaskbar, UnhookMessage, 0x0, 0x0);
	return TRUE;
}

// ##############################################################
// ############ RUNS IN OTHER THREAD / PROCESS ##################
// ##############################################################

HHOOK NewTaskbarHook = nullptr;

static LRESULT CALLBACK HookInject(int code, WPARAM wParam, LPARAM lParam)
{
	if (InjectionHookFirstRun)
	{
		InjectionHookFirstRun = false;

		HWND hwnd = ::FindWindow(nullptr, L"NativeMultiClock.HiddenDialog");
		::LoadLibrary(_T(PROJECT_TARGETNAME));
		if (hwnd != nullptr)
		{
			::PostMessage(hwnd, WM_UNHOOK_INJECTION, 0, 0);
		}

		EnumHandler handler = { HookTaskbar, nullptr };
		::EnumWindows(EnumAllTaskbarsEx, (LPARAM)&handler);

		::SetWindowSubclass(GetMainTaskbar(), MainTaskbarSubclassProc, 0, 0);
		NewTaskbarHook = ::SetWindowsHookEx(WH_CBT, HookNewTaskbar, GetModuleHandle(nullptr), GetCurrentThreadId());
	}
	return ::CallNextHookEx(nullptr, code, wParam, lParam);
}

static DWORD WINAPI FreeLibraryFunc(void *param)
{
	::Sleep(3000);
	::FreeLibraryAndExitThread(GlobalInstance, 0);
}

static LRESULT CALLBACK HookNewTaskbar(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HCBT_CREATEWND)
	{
		HWND newTaskbar = (HWND)wParam;
		wchar_t name[128];
		if (::GetClassName(newTaskbar, name, 128) != 0)
		{
			if (_wcsicmp(name, TASKBAR_SECONDARY_CLASS) == 0)
			{
				HWND mainTaskbar = GetMainTaskbar();
				if (mainTaskbar != nullptr)
				{
					::PostMessage(mainTaskbar, NewTaskbarMessage, (WPARAM)newTaskbar, 0x0);
				}
			}
		}
	}
	return CallNextHookEx(NULL, code, wParam, lParam);
}

static void CALLBACK HookTaskbar(HWND taskbar, VOID* unused)
{
	ClockWindow* multiClock = new ClockWindow();
	HWND window = multiClock->Create(taskbar, nullptr, nullptr, WS_CHILD | WS_DISABLED, WS_EX_LAYERED | WS_EX_TRANSPARENT, 0U, nullptr);
	if (window == nullptr)
	{
#if _DEBUG
		Beep(400, 100);
		DWORD error = GetLastError();
		if (error != 0)
		{
			MessageBoxError(error);
		}
#endif
		delete multiClock;
		return;
	}

	multiClock->RepositionIn(taskbar);
	multiClock->ShowWindow(SW_SHOW);
	multiClock->Refresh();
}

MYHOOK_API BOOL CALLBACK EnumAllTaskbarsEx(HWND hwnd, LPARAM lParam)
{
	wchar_t name[128];
	if (::GetClassName(hwnd, name, 128) == 0)
	{
		return TRUE;
	}

	if (_wcsicmp(name, TASKBAR_SECONDARY_CLASS) == 0)
	{
		EnumHandler* handler = (EnumHandler*)lParam;
		handler->func(hwnd, handler->param);
	}
	return TRUE;
}

static void CALLBACK CloseClock(HWND taskbar, VOID* unused)
{
	HWND hwnd = ::FindWindowEx(taskbar, nullptr, CLOCK_WINDOW_CLASS, nullptr);
	if (hwnd != nullptr)
	{
		::PostMessage(hwnd, WM_CLOSE, 0x0, 0x0);
	}
}

LRESULT CALLBACK MainTaskbarSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	if (uMsg == UnhookMessage)
	{
		EnumHandler handler = { CloseClock, nullptr };
		::EnumWindows(EnumAllTaskbarsEx, (LPARAM)&handler);
		::Sleep(1000);

		LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		::UnhookWindowsHookEx(NewTaskbarHook);
		::RemoveWindowSubclass(hWnd, MainTaskbarSubclassProc, uIdSubclass);
		::CreateThread(nullptr, 0, FreeLibraryFunc, nullptr, 0, nullptr);
		return result;
	}
	else if (uMsg == NewTaskbarMessage)
	{
		HookTaskbar((HWND)wParam);
	}
	else if (uMsg == WM_NCDESTROY)
	{
		::RemoveWindowSubclass(hWnd, MainTaskbarSubclassProc, uIdSubclass);
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}