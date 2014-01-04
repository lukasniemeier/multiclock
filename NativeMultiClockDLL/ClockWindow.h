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

#define CLOCK_WINDOW_CLASS L"NativeMultiClock.MyButton"

class Gdiplus::Graphics;

class ClockWindow : public CWindowImpl<ClockWindow>
{
public:
	DECLARE_WND_CLASS_EX(CLOCK_WINDOW_CLASS, CS_DBLCLKS, COLOR_MENU)

	BEGIN_MSG_MAP(ClockWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		/*MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_NCMOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WM_NCMOUSELEAVE, OnMouseLeave)*/
		MESSAGE_HANDLER(WM_INITMENU, OnInitMenu);
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()

	ClockWindow();

	void Refresh();
	LRESULT OnLeftButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLeftButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInitMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	void StartTrackingOn(HWND tray);
	void RepositionInTray(HWND tray);

	bool IsClicked() { return isClicked; }
public:
	static HWND GetOriginalClock();

protected:
	void DrawClockControl(Gdiplus::Graphics* graphics, int width, int height);
	void RenderTime(Gdiplus::Graphics* graphics, int width, int height);
	void RenderHighlight(Gdiplus::Graphics* graphics, int width, int height);
	void RenderClickedState(Gdiplus::Graphics* graphics, int width, int height);
	
	std::wstring GetTimeString(int maxHeight);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
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

