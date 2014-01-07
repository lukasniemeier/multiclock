#pragma once

#include <windows.h>

#ifdef NATIVEMULTICLOCKDLL_EXPORTS
#define MYHOOK_API __declspec(dllexport)
#else
#define MYHOOK_API __declspec(dllimport)
#endif

typedef VOID(CALLBACK* EnumHandlerFunc)(HWND parent, VOID* param);
#define WM_UNHOOK_INJECTION WM_USER + 0x05

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
MYHOOK_API BOOL Hook();
MYHOOK_API BOOL UnhookInjectionHook();
MYHOOK_API BOOL Unhook();

MYHOOK_API typedef struct
{
	EnumHandlerFunc func;
	VOID* param;
} EnumHandler;

MYHOOK_API BOOL CALLBACK EnumAllTaskbarsEx(HWND hwnd, LPARAM lParam);
void CALLBACK HookTaskbar(HWND tray, VOID* unused = nullptr);
void CALLBACK CloseClock(HWND tray, VOID* unused = nullptr);

LRESULT CALLBACK HookInject(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HookNewTaskbar(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TraySubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);