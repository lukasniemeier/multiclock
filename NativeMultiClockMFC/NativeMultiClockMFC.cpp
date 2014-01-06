#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "NativeMultiClockMFC.h"

#include "HiddenDialog.h"
#include "Hook.h"

#include <Shellapi.h>
#include <Shlwapi.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define HIDDEN_DIALOG_WINDOW_TEXT L"NativeMultiClock.HiddenDialog"

BEGIN_MESSAGE_MAP(CNativeMultiClockMFCApp, CWinApp)
END_MESSAGE_MAP()

CNativeMultiClockMFCApp theApp;

CNativeMultiClockMFCApp::CNativeMultiClockMFCApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	SetAppID(_T("NativeMultiClockMFC.AppID.NoVersion"));
}

bool TerminateOtherInstance()
{
	HWND other = ::FindWindow(nullptr, HIDDEN_DIALOG_WINDOW_TEXT);
	if (other != nullptr)
	{
		::SendMessage(other, WM_CLOSE, 0x0, 0x0);
		// Wait for the other process to terminate
		Sleep(5000);
	}
	other = ::FindWindow(nullptr, HIDDEN_DIALOG_WINDOW_TEXT);
	if (other != nullptr)
	{
		::MessageBox(NULL, L"Sorry but the other instance seems to be still running.\nPlease use the task manager to close all instances.", 
			L"Error", MB_OK | MB_ICONERROR);
		return true;
	}
	return true;
}

BOOL CNativeMultiClockMFCApp::InitInstance()
{
	appMutex = ::CreateMutex(nullptr, FALSE, L"NativeMultiClockMFC.AppMutex");
	if (appMutex == nullptr || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		int result = ::MessageBox(NULL, L"An Instance of this application is already running.\nI will try to close the other instance.", 
			L"Info", MB_OKCANCEL | MB_ICONINFORMATION);
		switch (result)
		{
			case IDCANCEL:
			return FALSE;
			case IDOK:
			{
				if (!TerminateOtherInstance())
				{
					return FALSE;
				}
				break;
			}
		}
	}

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	EnableTaskbarInteraction(FALSE);

	m_pMainWnd = CreateSysTrayDialog();
	if (m_pMainWnd == nullptr)
	{
		return FALSE;
	}
	return Hook();
}

CWnd* CNativeMultiClockMFCApp::CreateSysTrayDialog()
{
	HMODULE hInstance = GetModuleHandle(nullptr);

	HiddenDialog* dialog = new HiddenDialog();
	dialog->Create(dialog->IDD);
	dialog->SetWindowText(HIDDEN_DIALOG_WINDOW_TEXT);

	::ZeroMemory(&trayData, sizeof(NOTIFYICONDATA));
	trayData.cbSize = sizeof(NOTIFYICONDATA);
	trayData.uID = 42;
	trayData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	trayData.hIcon = (HICON) LoadImage(hInstance,
		MAKEINTRESOURCE(IDI_ICON1),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR);
	wsprintf(trayData.szTip, L"MultiClock");
	trayData.hWnd = dialog->m_hWnd;
	trayData.uCallbackMessage = WM_CUSTOM_TRAY_ICON;

	if (!::Shell_NotifyIcon(NIM_ADD, &trayData))
	{
		MessageBox(nullptr, L"Unable to set up notification area icon.", L"Error", MB_ICONERROR);
		return nullptr;
	}

	if (trayData.hIcon && ::DestroyIcon(trayData.hIcon))
	{
		trayData.hIcon = NULL;
	}

	return dialog;
}

int CNativeMultiClockMFCApp::ExitInstance()
{
	Shell_NotifyIcon(NIM_DELETE, &trayData);
	return Unhook() && CWinApp::ExitInstance();
}