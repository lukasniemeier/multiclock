#pragma once

#define TASKBAR_MAIN_CLASS L"Shell_TrayWnd"
#define TASKBAR_SECONDARY_CLASS L"Shell_SecondaryTrayWnd"
#define TASKBAR_WORKER_CLASS L"WorkerW"
#define TASKBAR_NOTIFICATION_AREA_CLASS L"TrayNotifyWnd"
#define ORIGINAL_CLOCK_CLASS L"TrayClockWClass"
#define ORIGINAL_CLOCK_FLYOUT_CLASS L"ClockFlyoutWindow"
#define CLOCK_WINDOW_CLASS L"NativeMultiClock.MyButton"

#include <windows.h>
#include <tchar.h>
#include <sstream>
typedef std::basic_stringstream<WCHAR> tstringstream;
template<typename T> tstringstream& operator,(tstringstream& tss, T t) { tss << _T(" ") << t; return tss; }
template<typename T> tstringstream& operator| (tstringstream& tss, const T t) { tss << t; return tss; }
#ifdef _DEBUG
#define OUTPUT_DEBUG_STRING(...) ::OutputDebugString((tstringstream() | L"[" | __FILE__ | L":" | __LINE__ | L"]" , __VA_ARGS__, L"\n").str().c_str()); 
#else
#define OUTPUT_DEBUG_STRING(...)
#endif
