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

BEGIN_MESSAGE_MAP(CNativeMultiClockMFCApp, CWinApp)
END_MESSAGE_MAP()

CNativeMultiClockMFCApp theApp;

CNativeMultiClockMFCApp::CNativeMultiClockMFCApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	SetAppID(_T("NativeMultiClockMFC.AppID.NoVersion"));
}

BOOL CNativeMultiClockMFCApp::InitInstance()
{
	appMutex = ::CreateMutex(nullptr, FALSE, L"NativeMultiClockMFC.AppMutex");
	if (appMutex == nullptr || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		::MessageBox(NULL, _T("An Instance of this application is already running..."), _T("Info"), 
			MB_OK | MB_ICONINFORMATION);
		return FALSE;
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