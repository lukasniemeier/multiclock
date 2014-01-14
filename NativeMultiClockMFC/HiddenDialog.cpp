// HiddenDialog.cpp : implementation file
//

#include "stdafx.h"
#include "NativeMultiClockMFC.h"
#include "HiddenDialog.h"
#include "afxdialogex.h"

#include "Hook.h"
#include "ClockWindow.h"

// HiddenDialog dialog

IMPLEMENT_DYNAMIC(HiddenDialog, CDialog)

HiddenDialog::HiddenDialog(CWnd* pParent /*=NULL*/)
	: CDialog(HiddenDialog::IDD, pParent)
{
	isCreated = false;
}

HiddenDialog::~HiddenDialog()
{
}

void HiddenDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

static UINT TaskbarCreated = RegisterWindowMessage(TEXT("TaskbarCreated"));

BEGIN_MESSAGE_MAP(HiddenDialog, CDialog)
	ON_MESSAGE(WM_CUSTOM_TRAY_ICON, &HiddenDialog::OnCustomTrayIcon)
	ON_MESSAGE(WM_CLOSE, &HiddenDialog::OnClose)
	ON_MESSAGE(WM_DESTROY, &HiddenDialog::OnDestroy)
	ON_MESSAGE(WM_UNHOOK_INJECTION, &HiddenDialog::OnUnhookInjection)
	ON_WM_CREATE()
	ON_REGISTERED_MESSAGE(TaskbarCreated, &HiddenDialog::OnTaskbarCreated)
END_MESSAGE_MAP()

void HiddenDialog::ShowContextMenu()
{
	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{
		InsertMenu(hMenu, 0, MF_BYPOSITION, 0x41, L"Exit");
		SetForegroundWindow();
		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, *this, NULL);
		DestroyMenu(hMenu);
	}
}

static void CALLBACK CountClock(HWND tray, VOID* param)
{
	HWND hwnd = ::FindWindowEx(tray, nullptr, CLOCK_WINDOW_CLASS, nullptr);
	if (hwnd != nullptr)
	{
		(*((int*)param))++;
	}
}

int HiddenDialog::GetClockCount()
{
	int count = 0;
	EnumHandler handler = { CountClock, &count };
	::EnumWindows(EnumAllTaskbarsEx, (LPARAM)&handler);
	return count;
}

bool HiddenDialog::SetupNotification()
{
	::ZeroMemory(&notificationData, sizeof(NOTIFYICONDATA));
	notificationData.cbSize = sizeof(NOTIFYICONDATA);
	notificationData.uID = 0x41;
	notificationData.uVersion = NOTIFYICON_VERSION_4;
	notificationData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
	notificationData.hIcon = (HICON)LoadImage(GetModuleHandle(nullptr),
		MAKEINTRESOURCE(IDI_ICON1),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR);
	notificationData.hWnd = *this;
	notificationData.uCallbackMessage = WM_CUSTOM_TRAY_ICON;
	wsprintf(notificationData.szTip, L"MultiClock");

	BOOL success = ::Shell_NotifyIcon(NIM_ADD, &notificationData);
	if (success)
	{
		success = ::Shell_NotifyIcon(NIM_SETVERSION, &notificationData);
	}
	return success == TRUE;
}

int HiddenDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	while (!SetupNotification())
	{
		int result = ::MessageBox(nullptr, 
			L"The shell (explorer.exe) must be running before starting this program.", 
			L"MultiClock", MB_RETRYCANCEL | MB_ICONERROR);
		if (result == IDCANCEL)
		{
			return -1;
		}
	}
	isCreated = true;

	return 0;
}

afx_msg LRESULT HiddenDialog::OnTaskbarCreated(WPARAM wParam, LPARAM lParam)
{
	while (isCreated && !SetupNotification())
	{
		int result = ::MessageBox(nullptr, L"Unable to set up notification area icon.", L"MultiClock", MB_RETRYCANCEL | MB_ICONERROR);
		if (result == IDCANCEL)
		{
			DestroyWindow();
			return 0;
		}
	}
	Hook();
	return 0;
}


afx_msg LRESULT HiddenDialog::OnCustomTrayIcon(WPARAM wParam, LPARAM lParam)
{
	int clockCount;
	bool expertMenu = false;
	switch (LOWORD(lParam))
	{
	case WM_MOUSEMOVE:
		clockCount = HiddenDialog::GetClockCount();
		if (clockCount == 0)
		{
			swprintf(notificationData.szTip, 128, L"MultiClock (no clock shown)");
		}
		else
		{
			swprintf(notificationData.szTip, 128, L"MultiClock (%d clock%s shown)", HiddenDialog::GetClockCount(), clockCount == 1 ? L"" : L"s");
		}
		::Shell_NotifyIcon(NIM_MODIFY, &notificationData);
		break;
	case WM_LBUTTONDBLCLK:
		::PostQuitMessage(0);
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
	case WM_CONTEXTMENU:
	{
		ShowContextMenu();
	}
	default:
		break;
	}
	return 0;
}

afx_msg LRESULT HiddenDialog::OnClose(WPARAM wParam, LPARAM lParam)
{
	return DestroyWindow(); 
}

afx_msg LRESULT HiddenDialog::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	if (notificationData.hIcon && ::DestroyIcon(notificationData.hIcon))
	{
		notificationData.hIcon = NULL;
	}
	::Shell_NotifyIcon(NIM_DELETE, &notificationData);
	::PostQuitMessage(0);
	return 0;
}


BOOL HiddenDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	if (wmId == 0x41)
	{
		return DestroyWindow();;
	}
	return CDialog::OnCommand(wParam, lParam);
}


afx_msg LRESULT HiddenDialog::OnUnhookInjection(WPARAM wParam, LPARAM lParam)
{
	return UnhookInjectionHook();
}