#pragma once

#include <windows.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include <atlbase.h>
#include <atltypes.h>
#include <atlstr.h>
#include <atlwin.h>

#include <objidl.h>
#include <gdiplus.h>

#include <Strsafe.h>
#include <string>

#include "Globals.h"

// Used for tooltips
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

class Gdiplus::Graphics;

class ClockWindow : public CWindowImpl<ClockWindow>
{
public:

	enum Orientation 
	{
		BOTTOM,
		TOP,
		LEFT,
		RIGHT
	};

	DECLARE_WND_CLASS_EX(CLOCK_WINDOW_CLASS, CS_DBLCLKS, COLOR_MENU)

	BEGIN_MSG_MAP(ClockWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_INITMENU, OnInitMenu);
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	ClockWindow();

	virtual void OnFinalMessage(HWND hwnd);

	void Refresh(bool force = false);
	void RepositionIn(HWND taskbar);
	void StartTrackingOn(HWND taskbar);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLeftButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLeftButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInitMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	bool IsClicked() { return isClicked; }
public:
	static HWND GetOriginalClock();
	static Orientation GetTaskbarOrientation(HWND hwnd);
	
protected:
	void RenderTime(HDC context, int width, int height) const;
	void RenderHighlighting(Gdiplus::Graphics* graphics, int width, int height) const;
	void RenderHighlight(Gdiplus::Graphics* graphics, int width, int height) const;
	void RenderClickedState(Gdiplus::Graphics* graphics, int width, int height) const;

	bool IsVisible(const RECT& clientRect);
	
	void MoveClockFlyout() const;

private:
	static LONG64 CurrentSubclassProcId;

	UINT_PTR taskbarSubclassProcId;
	UINT_PTR workerWSubclassProcId;
	UINT_PTR originalClockSubclassProcId;

	ULONG_PTR gdiplusToken;
	HWND toolTipWindow;
	TOOLINFOW toolTipInfo;
	TRACKMOUSEEVENT trackMouseEventInfo;
	bool isHighlighted;
	bool isClicked;


protected:
	static LRESULT CALLBACK TaskbarSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
	static LRESULT CALLBACK WorkerWSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
	static LRESULT CALLBACK OriginalClockSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

};

