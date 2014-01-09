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

	INITCOMMONCONTROLSEX commonControls;
	commonControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	commonControls.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
	InitCommonControlsEx(&commonControls);

	CWinApp::InitInstance();
	EnableTaskbarInteraction(FALSE);

	m_pMainWnd = CreateNotificationDialog();
	if (m_pMainWnd == nullptr)
	{
		return FALSE;
	}
	return Hook();
}

CWnd* CNativeMultiClockMFCApp::CreateNotificationDialog()
{
	HMODULE hInstance = GetModuleHandle(nullptr);

	HiddenDialog* dialog = new HiddenDialog();
	dialog->Create(dialog->IDD);
	dialog->SetWindowText(HIDDEN_DIALOG_WINDOW_TEXT);

	::ZeroMemory(&notificationData, sizeof(NOTIFYICONDATA));
	notificationData.cbSize = sizeof(NOTIFYICONDATA);
	notificationData.uID = 42;
	notificationData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notificationData.hIcon = (HICON)LoadImage(hInstance,
		MAKEINTRESOURCE(IDI_ICON1),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR);
	wsprintf(notificationData.szTip, L"MultiClock");
	notificationData.hWnd = dialog->m_hWnd;
	notificationData.uCallbackMessage = WM_CUSTOM_TRAY_ICON;

	if (!::Shell_NotifyIcon(NIM_ADD, &notificationData))
	{
		MessageBox(nullptr, L"Unable to set up notification area icon.", L"Error", MB_ICONERROR);
		return nullptr;
	}

	if (notificationData.hIcon && ::DestroyIcon(notificationData.hIcon))
	{
		notificationData.hIcon = NULL;
	}

	return dialog;
}

int CNativeMultiClockMFCApp::ExitInstance()
{
	Shell_NotifyIcon(NIM_DELETE, &notificationData);
	return Unhook() && CWinApp::ExitInstance();
}