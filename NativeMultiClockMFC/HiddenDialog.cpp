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

}

HiddenDialog::~HiddenDialog()
{
}

void HiddenDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(HiddenDialog, CDialog)
	ON_MESSAGE(WM_CUSTOM_TRAY_ICON, &HiddenDialog::OnCustomTrayIcon)
END_MESSAGE_MAP()

void HiddenDialog::ShowContextMenu(bool expert)
{
	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{
		InsertMenu(hMenu, 0, MF_BYPOSITION, 0x41, L"Exit");
		if (expert)
		{
			wchar_t text[128];
			int clockCount = HiddenDialog::GetClockCount();
			if (clockCount == 0)
			{
				swprintf(text, 128, L"No clock shown");
			}
			else
			{
				swprintf(text, 128, L"%d clock%s shown", 
					HiddenDialog::GetClockCount(), clockCount == 1 ? L"" : L"s");
			}

			MENUITEMINFO info;
			info.cbSize = sizeof(MENUITEMINFO);
			info.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
			info.wID = 0x42;
			info.fType = MFT_STRING;
			info.fState = MFS_DISABLED;
			info.dwTypeData = text;
			info.cch = 16;

			InsertMenuItem(hMenu, 0, MF_BYPOSITION, &info);
		}
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

afx_msg LRESULT HiddenDialog::OnCustomTrayIcon(WPARAM wParam, LPARAM lParam)
{
	bool expertMenu = false;
	switch (lParam)
	{
	case WM_LBUTTONDBLCLK:
		::PostQuitMessage(0);
		break;
	case WM_RBUTTONDOWN:
		expertMenu = true;
	case WM_LBUTTONDOWN:
	case WM_CONTEXTMENU:
	{
		ShowContextMenu(expertMenu);
	}
	default:
		break;
	}
	return 0;
}


BOOL HiddenDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	if (wmId == 0x41)
	{
		::PostQuitMessage(0);
		return TRUE;
	}
	return CDialog::OnCommand(wParam, lParam);
}
