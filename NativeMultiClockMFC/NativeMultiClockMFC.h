#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"

#define WM_CUSTOM_TRAY_ICON (WM_USER + 42)

#include "HiddenDialog.h"
#include "Hook.h"

#include <Shellapi.h>
#include <Shlwapi.h>


class CNativeMultiClockMFCApp : public CWinApp
{
public:
	CNativeMultiClockMFCApp();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	HANDLE appMutex;
	HiddenDialog dialog;

public:
	DECLARE_MESSAGE_MAP()
};

extern CNativeMultiClockMFCApp theApp;
