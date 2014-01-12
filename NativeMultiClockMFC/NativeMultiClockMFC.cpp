#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "NativeMultiClockMFC.h"


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
		::SendMessage(other, WM_DESTROY, 0x0, 0x0);
		// Wait for the other process to terminate
		Sleep(5000);
	}
	other = ::FindWindow(nullptr, HIDDEN_DIALOG_WINDOW_TEXT);
	if (other != nullptr)
	{
		::MessageBox(NULL, L"Sorry but the other instance seems to be still running.\nPlease use the task manager to close all instances.", 
			L"Error", MB_OK | MB_ICONERROR);
		return false;
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

	//HiddenDialog* dialog = new HiddenDialog();
	if (!dialog.Create(dialog.IDD))
	{
		return FALSE;
	}
	dialog.SetWindowText(HIDDEN_DIALOG_WINDOW_TEXT);

	m_pMainWnd = &dialog;
	if (m_pMainWnd == nullptr)
	{
		return FALSE;
	}
	return Hook();
}

int CNativeMultiClockMFCApp::ExitInstance()
{
	BOOL unhooked = Unhook();
	if (unhooked == FALSE)
	{
		::MessageBox(nullptr, L"Removing the additional clocks failed.", L"Error", MB_OK);
	}
	return CWinApp::ExitInstance();
}